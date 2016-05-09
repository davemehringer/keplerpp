/*
 * JPLHorizonsRetriever.cpp
 *
 *  Created on: Sep 2, 2015
 *      Author: dmehring
 */

#include <SolarSystem/JPLHorizonsRetriever.h>

#include  <Exception/KeplerException.h>
#include <IO/IO.h>
#include <Math/Constants.h>
#include <Math/Vector.h>
#include <Physics/Body.h>
#include <Physics/StateVectorsOrbitalElements.h>
#include <SolarSystem/SSObjects.h>

#include <fstream>
#include <future>
#include <iomanip>
#include <iterator>
#include <iostream>
#include <sstream>
#include <thread>
#include <sys/stat.h>

using namespace std;

namespace kepler {

const map<int, string> JPLHorizonsRetriever::_refPlaneString {
	{ (int)ECLIPTIC, "ECLIP"},
	{ (int)BODY, "BODY"}
};

const map<int, string> JPLHorizonsRetriever::_outTypeString {
	{ (int)ELEMENTS, "OE"},
	{ (int)VECTORS, "SV"}
};

JPLHorizonsRetriever::JPLHorizonsRetriever(OutputType type) :
	_outputType(type), _bodyStruct() {}

JPLHorizonsRetriever::~JPLHorizonsRetriever() {}

void JPLHorizonsRetriever::add(
	const string& bodyName, const string& centerBody,
	PrecType time, RefPlane refPlane
) {
	static const string dir = "/tmp/kepler++";
	if (! _exists(dir)) {
		auto  stat = mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		if (stat == -1) {
			throw KeplerException("Error creating directory " + dir);
		}
	}
	string rp = _refPlaneString.find(refPlane)->second;
	string ot = _outTypeString.find(_outputType)->second;
	ostringstream oss;
	oss << bodyName << "_" << centerBody << "_" << setprecision(20) << std::fixed
		<< time << "_" << rp << "_" << ot;
	string base = dir +  "/" + oss.str();
	string inputFilename =  base + ".inp";
	string outputFilename = base + ".out";
	BodyStruct bs;
	bs.name = bodyName;
	bs.centerBody = centerBody;
	bs.time = time;
	bs.refPlane = refPlane;
	bs.inputFile = inputFilename;
	bs.outputFile = outputFilename;
	bs.jplID = SSObjects::jplID(bodyName);
	_bodyStruct.push_back(bs);
	if (_exists(inputFilename)) {
		return;
	}
	ofstream myfile;
	myfile.open(inputFilename.c_str());
	if (! myfile) {
		throw KeplerException(
			"Unable to open file " + inputFilename
			+ " for writing"
		);
	}
	myfile << " set   EMAIL_ADDR           \"\"" << endl;
	myfile << " set   CENTER               \"@" << SSObjects::jplID(centerBody) << "\"" << endl;
	myfile << " set   REF_PLANE            \"" << rp
		<< "\"" << endl;
	myfile << " set   START_TIME           \"JD " << setprecision(20) << std::fixed  << time << "\"" << endl;
	myfile << " set   STOP_TIME            \"JD " << setprecision(20)  << std::fixed << (time+1) << "\"" << endl;
	myfile << " set   STEP_SIZE            \"2d\"" << endl;
	myfile.close();
}

vector<Body> JPLHorizonsRetriever::getBodies() const {
	return _bodies;
}

vector<Elements> JPLHorizonsRetriever::getElements() const {
    return _elements;
}

int JPLHorizonsRetriever::_staticRetrieve(const string& cmd) {
	auto ret = system(cmd.c_str());
	return ret;
}

bool JPLHorizonsRetriever::_exists(const string& filename) {
	struct stat buffer;
	return (stat (filename.c_str(), &buffer) == 0);
}

void JPLHorizonsRetriever::retrieve() {
	vector<future<int> > cmdRet;
	vector<string> cmd;
	int maxTasks = 8;
	string script = _outputType == VECTORS
		? "state_tbl" : "osc_tbl";
	string msg = _outputType == VECTORS
		? "state vectors" : "orbital elements";
	script = "./" + script;
	std::chrono::milliseconds span (100);
	int nTasks = 0;
	for (const auto& bs :_bodyStruct) {
		if (_exists(bs.outputFile)) {
			//cout << "Found " <<bs.outputFile << endl;
			continue;
		}
		cout << "Retrieve " << msg << " for " << bs.name
			<< " relative to " << bs.centerBody << " at JD "
			<< bs.time << " using ref plane "
			<< _refPlaneString.find(bs.refPlane)->second << endl;
		ostringstream oss;
		oss << script << " " << bs.jplID << " "
			<< bs.inputFile << " " << bs.outputFile;
		string c = oss.str();
		cmd.push_back(c);
		cout << c << endl;
		cmdRet.push_back(
			async(
				launch::async, kepler::JPLHorizonsRetriever::_staticRetrieve, c
			)
		);
		++nTasks;
		if (nTasks >= maxTasks) {
			bool startNewThread = false;
			while (! startNewThread) {
				int activeCount = 0;
				for (const auto& cr : cmdRet) {
					if (cr.wait_for(span)==std::future_status::timeout) {
						++activeCount;
						if (activeCount >= maxTasks) {
							break;
						}
					}
				}
				if (activeCount < maxTasks) {
					startNewThread = true;
				}
			}
		}
	}
	int i = 0;
	for (auto& retVal : cmdRet) {
		int code = retVal.get();
		if (code != 0) {
			throw KeplerException("Bad return value from " + cmd[i]);
		}
	}
	if (_outputType == VECTORS) {
		_doBodies();
	}
	else {
	    _doElements();
	}
}

void JPLHorizonsRetriever::_doBodies() {
	static const regex spaces("\\s+");
	for (const auto& bs : _bodyStruct) {
		string file = bs.outputFile;
		auto goodLines = _getLines(file, 3);
		auto xtokens = split(goodLines[1], spaces);
		auto vtokens = split(goodLines[2], spaces);
		Vector x, v;
		for (auto i=0; i<3; ++i) {
	        x[i] = stod(xtokens[i]);
	        v[i] = stod(vtokens[i]);
		}
		auto body = SSObjects::createBody(bs.name);
		body.x = KM_PER_AU*x;
		body.v = KMPERSEC_PER_AUPERDAY*v;
		_bodies.push_back(body);
	}
}

void JPLHorizonsRetriever::_doElements() {
    static const regex spaces("\\s+");
    static const regex seq("\\s*=\\s*");
    static const regex eq("=");
    for (const auto& bs : _bodyStruct) {
        string file = bs.outputFile;
        auto goodLines = _getLines(file, 5);
        // The first line is the time stamp so can be erased.
        goodLines.erase(goodLines.begin());
        map<string, string> mymap;
        for (auto line : goodLines) {
            line = regex_replace(line, seq, "=");
            auto kvs = split(line, spaces);
            for (auto kv: kvs) {
                kv = trim(kv);
                auto s = split(kv, eq);
                mymap[s[0]] = s[1];
            }
        }
        Elements el;
        el.a = stod(mymap["A"]);
        el.e = stod(mymap["EC"]);
        el.i = stod(mymap["IN"]);
        el.peri = stod(mymap["W"]);
        el.node = stod(mymap["OM"]);
        el.mAnomaly = stod(mymap["MA"]);
        _elements.push_back(el);
    }
}

vector<string> JPLHorizonsRetriever::_getLines(
	const string& file, int count
) {
	ifstream myfile (file);
	if (! myfile.is_open()) {
		throw KeplerException("Unable to open file " + file);
	}
	string line;
	int lineNumber = -1;
	vector<string> goodLines;
	static const string e("$$SOE");
	while (getline(myfile, line)) {
		if (lineNumber >= 0) {
			goodLines.push_back(trim(line));
			++lineNumber;
			if (lineNumber == count) {
				myfile.close();
				return goodLines;
			}
		}
		else if (line == e) {
			lineNumber = 0;
		}
	}
	// make eclipse code assist happy, this statement should never be reached
	return vector<string>();
}

}