#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include <vector>

class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();
	void ofApp::keyPressed(int key);

	int readSettingFile();
	int connectToMySQL();
	int readTable(vector<vector<string>>& tableData_);
	int sendOscPacket(vector<vector<string>> sendData, bool testMode = false);
	int updateTable(vector<vector<string>> sendData);
	int createTable();
	int insertData();
	int resetTable();

	string getCurrentDate();
	string getCurrentTime();

	int timeThen = 0;

	vector<vector<string>> tableData;

	ofxOscSender snd;
	ofImage img;

private:
	int READ_INTERVAL_SEC;
	int FW_LAUNCH_LIMIT;
	int DB_RESET_ENABLE;
	int PORT_TO_FW_APP;
	int PC_ID;
	string FW_APP_IP;
	string TABLENAME;
	string USERNAME;
	string PASSWORD;
	string HOSTNAME;
	string DBNAME;

	int chk;
	bool connectionEstablished;
};