#include "ofApp.h"
#include "define.h"
#include "confReader.hpp"

#include <stdlib.h>
#include <iostream>
//#include "stdafx.h"

#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>

using namespace std;
using namespace sql;

Driver* driver;
Connection* con;
Statement* stmt;
PreparedStatement* pstmt;
ResultSet *result;

//--------------------------------------------------------------
void ofApp::setup() {

	// Read setting file
	chk = readSettingFile();
	if (chk == int(PROCESS_RESULT::READ_SETTINGFILE_ERROR)) {
		printf("%s %s Setting file read error\n",
			getCurrentDate().c_str(),
			getCurrentTime().c_str());
	}

	// Connect to MySQL
	chk = connectToMySQL();
	if (chk == int(PROCESS_RESULT::CONNECTION_ERROR)) {
		printf("%s %s Could not connect to server\n",
			getCurrentDate().c_str(),
			getCurrentTime().c_str());
		connectionEstablished = false;
	}
	else {
		connectionEstablished = true;
	}

	// Reset table
	if (DB_RESET_ENABLE) {
		chk = resetTable();
		if (chk == int(PROCESS_RESULT::RESET_TABLE_ERROR)) {
			printf("%s %s Table reset error\n",
				getCurrentDate().c_str(),
				getCurrentTime().c_str());
		}
	}

	// Load image file
	chk = img.load("database.png");
	if (chk == false) {
		printf("%s %s Load image error\n",
			getCurrentDate().c_str(),
			getCurrentTime().c_str());
	}

	// Set osc port
	chk = snd.setup(FW_APP_IP, PORT_TO_FW_APP);
	if (chk == false) {
		printf("%s %s Osc setting error\n",
			getCurrentDate().c_str(),
			getCurrentTime().c_str());
	}

	// Send test msg
	chk = sendOscPacket(tableData, true);
	if (chk == int(PROCESS_RESULT::OSC_WRITE_ERROR)) {
		printf("%s %s Osc test msg error\n",
			getCurrentDate().c_str(),
			getCurrentTime().c_str());
	}

	printf("%s %s Settings complete!\n",
		getCurrentDate().c_str(),
		getCurrentTime().c_str());

	//createTable();
	//insertData();
	//insertData();
	//insertData();
}

//--------------------------------------------------------------
void ofApp::update() {

	if ((clock() - timeThen) > READ_INTERVAL_SEC * 1000) {
		chk = readTable(tableData);
		if (chk == int(PROCESS_RESULT::READ_TABLE_ERROR)) {
			connectionEstablished = false;
			printf("%s %s Read table error\n",
				getCurrentDate().c_str(),
				getCurrentTime().c_str());
		}

		chk = sendOscPacket(tableData);
		if (chk == int(PROCESS_RESULT::OSC_WRITE_ERROR)) {
			printf("%s %s Osc write error\n",
				getCurrentDate().c_str(),
				getCurrentTime().c_str());
		}

		chk = updateTable(tableData);
		if (chk == int(PROCESS_RESULT::UPDATE_TABLE_ERROR)) {
			connectionEstablished = false;
			printf("%s %s Update table error\n",
				getCurrentDate().c_str(),
				getCurrentTime().c_str());
		}

		if (connectionEstablished == false) {
			// Re-connect to MySQL
			chk = connectToMySQL();
			if (chk == int(PROCESS_RESULT::CONNECTION_ERROR)) {				
				printf("%s %s Failed to re-connect to server\n",
					getCurrentDate().c_str(),
					getCurrentTime().c_str());
			}
			else {
				connectionEstablished = true;
			}
		}

		timeThen = clock();
	}
}

//--------------------------------------------------------------
void ofApp::draw() {
	img.draw(22, 22);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key == 't') {
		// Send test msg
		chk = sendOscPacket(tableData, true);
		if (chk == int(PROCESS_RESULT::OSC_WRITE_ERROR)) {
			printf("%s %s Osc test msg error\n",
				getCurrentDate().c_str(),
				getCurrentTime().c_str());
		}
	}
}

//--------------------------------------------------------------
int ofApp::readSettingFile() {

	//-- Load setting file --
	ifstream infile("bin/data/setting.conf");
	string separator = conf::setSeparetor(",", "=", "\\s");

	if (infile) {
		conf::setMap(conf::config_map, infile, separator);
	}
	else {
		return int(PROCESS_RESULT::READ_SETTINGFILE_ERROR);
	}

	//-- Load setting value --
	READ_INTERVAL_SEC = conf::readMap("READ_INTERVAL_SEC");
	FW_LAUNCH_LIMIT = conf::readMap("FW_LAUNCH_LIMIT");
	PORT_TO_FW_APP = conf::readMap("PORT_TO_FW_APP");
	PC_ID = conf::readMap("PC_ID");
	DB_RESET_ENABLE = conf::readMap("DB_RESET_ENABLE");
	string fwAppIp = conf::readMap("FW_APP_IP"); FW_APP_IP = fwAppIp;
	string userName = conf::readMap("USERNAME"); USERNAME = userName;
	string password = conf::readMap("PASSWORD"); PASSWORD = password;
	string hostName = conf::readMap("HOSTNAME"); HOSTNAME = hostName;
	string dbName = conf::readMap("DBNAME");   DBNAME = dbName;
	string tableName = conf::readMap("TABLENAME"); TABLENAME = tableName;

	//-- Show setting value --
	printf("------ SETTING VALUES ------\n");
	printf("READ_INTERVAL_SEC = %d\n", READ_INTERVAL_SEC);
	printf("FW_LAUNCH_LIMIT = %d\n", FW_LAUNCH_LIMIT);
	printf("PORT_TO_FW_APP = %d\n", PORT_TO_FW_APP);
	printf("FW_APP_IP = %s\n", FW_APP_IP.c_str());
	printf("PC_ID = %d\n", PC_ID);
	printf("DB_RESET_ENABLE = %d\n", DB_RESET_ENABLE);
	printf("USERNAME = %s\n", USERNAME.c_str());
	printf("PASSWORD = %s\n", PASSWORD.c_str());
	printf("HOSTNAME = %s\n", HOSTNAME.c_str());
	printf("DBNAME = %s\n", DBNAME.c_str());
	printf("TABLENAME = %s\n", TABLENAME.c_str());
	printf("---------------------------\n");

	return int(PROCESS_RESULT::PROCESS_SUCCESS);
}

//--------------------------------------------------------------
int ofApp::updateTable(vector<vector<string>> tableData_) {

	for (int i = 0; i < tableData_.size(); i++) {
		//-- Create query --
		string query = "UPDATE " + TABLENAME + " SET LAUNCHED = 1 WHERE JOB_ID = " + tableData_[i].at(int(DATA_SET::JB_ID)) + ";";
		//cout << "QUERY= " << query << endl;

		//-- Execute query --
		try {
			pstmt = con->prepareStatement(query);
			result = pstmt->executeQuery();
		}
		catch (sql::SQLException e) {
			cout << e.what() << endl;
			return int(PROCESS_RESULT::UPDATE_TABLE_ERROR);
		}
	}

	return int(PROCESS_RESULT::PROCESS_SUCCESS);
}

//--------------------------------------------------------------
int ofApp::connectToMySQL() {

	printf("%s %s Connecting...\n",
		getCurrentDate().c_str(),
		getCurrentTime().c_str());

	try {
		driver = get_driver_instance();
		con = driver->connect(HOSTNAME, USERNAME, PASSWORD);
		printf("%s %s Connection succeed!\n",
			getCurrentDate().c_str(),
			getCurrentTime().c_str());
	}
	catch (sql::SQLException e) {
		cout << e.what() << endl;
		return int(PROCESS_RESULT::CONNECTION_ERROR);
	}

	con->setSchema(DBNAME);

	return int(PROCESS_RESULT::PROCESS_SUCCESS);
}

//--------------------------------------------------------------
int ofApp::readTable(vector<vector<string>>& tableData_) {

	vector<string> tmp;
	tableData_.clear();

	//-- Create query --
	string query = "SELECT * FROM " + TABLENAME + " WHERE PC_ID = " + to_string(PC_ID) + " AND LAUNCHED = 0 LIMIT " + to_string(FW_LAUNCH_LIMIT);
	//cout << "QUERY= " << query << endl;

	//-- Execute query --
	try {
		pstmt = con->prepareStatement(query);
		result = pstmt->executeQuery();
	}
	catch (sql::SQLException e) {
		cout << e.what() << endl;
		return int(PROCESS_RESULT::READ_TABLE_ERROR);
	}

	//データ参照
	while (result->next()) {
		tmp.clear();
		tmp.push_back(result->getString(int(DATA_SET::JB_ID) + 1)); //JOB_ID
		tmp.push_back(result->getString(int(DATA_SET::PC_ID) + 1)); //PC_ID
		tmp.push_back(result->getString(int(DATA_SET::TP_ID) + 1)); //TYPE_ID
		tmp.push_back(result->getString(int(DATA_SET::LUNCH) + 1)); //LAUNCHED
		tmp.push_back(result->getString(int(DATA_SET::UPDAT) + 1)); //UPDATED_AT
		tableData_.push_back(tmp);

		/*
		printf("Reading from table = (JB=%s, PC=%s, TYPE=%s, LUNCH=%s, UPDATE_AT=%s)\n",
			tmp[int(DATA_SET::JB_ID)].c_str(),
			tmp[int(DATA_SET::PC_ID)].c_str(),
			tmp[int(DATA_SET::TP_ID)].c_str(),
			tmp[int(DATA_SET::LUNCH)].c_str(),
			tmp[int(DATA_SET::UPDAT)].c_str());
		*/
	};

	return int(PROCESS_RESULT::PROCESS_SUCCESS);
}

//--------------------------------------------------------------
int ofApp::sendOscPacket(vector<vector<string>> sendData, bool testMode) {

	ofxOscMessage m;

	if (testMode == false) {

		for (int i = 0; i < sendData.size(); i++) {
			m.setAddress("/fireWorks/launchData");

			//m.addIntArg(stoi(sendData[i].at(int(DATA_SET::JB_ID)))); //JOB_ID
			//m.addIntArg(stoi(sendData[i].at(int(DATA_SET::PC_ID)))); //PC_ID
			m.addIntArg(stoi(sendData[i].at(int(DATA_SET::TP_ID)))); //TYPE_ID
			//m.addIntArg(stoi(sendData[i].at(int(DATA_SET::LUNCH)))); //LAUNCHED
			//UPDATED_ATは送信しない

			snd.sendMessage(m);
			printf("%s %s send data = ID=%s, PC=%s, TYPE=%s, LAUNCHED=%s, UPDATED_AT=%s\n",
				getCurrentDate().c_str(),
				getCurrentTime().c_str(),
				sendData[i].at(int(DATA_SET::JB_ID)).c_str(),
				sendData[i].at(int(DATA_SET::PC_ID)).c_str(),
				sendData[i].at(int(DATA_SET::TP_ID)).c_str(),
				sendData[i].at(int(DATA_SET::LUNCH)).c_str(),
				sendData[i].at(int(DATA_SET::UPDAT)).c_str());
		}
	}
	else {
		m.setAddress("/fireWorks/testMessage");
		snd.sendMessage(m);
		printf("%s %s Send test msg\n",
			getCurrentDate().c_str(),
			getCurrentTime().c_str());
	}

	return int(PROCESS_RESULT::PROCESS_SUCCESS);
}

//--------------------------------------------------------------
string ofApp::getCurrentDate() {
	time_t timeValue;
	struct tm *timeObject;
	stringstream currentTime;

	time(&timeValue);
	timeObject = localtime(&timeValue);

	int Y = timeObject->tm_year - 100; //20XX年 >> XX
	int M = timeObject->tm_mon + 1;
	int D = timeObject->tm_mday;

	// setw(),setfill()で0詰め
	currentTime << setw(2) << setfill('0') << Y << "/";
	currentTime << setw(2) << setfill('0') << M << "/";
	currentTime << setw(2) << setfill('0') << D;

	return currentTime.str();
}

//--------------------------------------------------------------
string ofApp::getCurrentTime() {
	time_t timeValue;
	struct tm *timeObject;
	stringstream currentTime;

	time(&timeValue);
	timeObject = localtime(&timeValue);

	int h = timeObject->tm_hour;
	int m = timeObject->tm_min;
	int s = timeObject->tm_sec;

	// setw(),setfill()で0詰め
	currentTime << setw(2) << setfill('0') << h << ":";
	currentTime << setw(2) << setfill('0') << m << ":";
	currentTime << setw(2) << setfill('0') << s;

	return currentTime.str();
}

//--------------------------------------------------------------
int ofApp::createTable() {

	//-- Create query --
	string query = "CREATE TABLE fireWorksTable (JOB_ID INT(11) not null AUTO_INCREMENT PRIMARY KEY, PC_ID INT(11) not null, TYPE_ID INT(11) not null, LAUNCHED BOOLEAN not null, UPDATED_AT TIMESTAMP not null) engine = innodb default charset = utf8";
	//cout << "QUERY= " << query << endl;

	//-- Execute query --
	try {
		pstmt = con->prepareStatement(query);
		result = pstmt->executeQuery();
		printf("%s %s DB table created!\n",
			getCurrentDate().c_str(),
			getCurrentTime().c_str());
	}
	catch (sql::SQLException e) {
		cout << "<Error> in createTable(). Error message: " << e.what() << endl;
		return int(PROCESS_RESULT::DB_CREATE_ERROR);
	}
}

//--------------------------------------------------------------
int ofApp::insertData() {

	int type = ofRandom(0, 6);

	//-- Create query --
	string query = "insert INTO fireworkstable (PC_ID, TYPE_ID, LAUNCHED) VALUES (0," + to_string(type) + ",0);";
	//cout << "QUERY= " << query << endl;

	//-- Execute query --
	try {
		pstmt = con->prepareStatement(query);
		result = pstmt->executeQuery();
		printf("%s %s DB inserted\n",
			getCurrentDate().c_str(),
			getCurrentTime().c_str());
	}
	catch (sql::SQLException e) {
		cout << "<Error> in insertData(). Error message: " << e.what() << endl;
		return int(PROCESS_RESULT::DB_CREATE_ERROR);
	}
}

//--------------------------------------------------------------
int ofApp::resetTable() {

	//-- Create query --
	string query = "TRUNCATE table " + TABLENAME;
	//cout << "QUERY= " << query << endl;

	//-- Execute query --
	try {
		pstmt = con->prepareStatement(query);
		result = pstmt->executeQuery();
		printf("%s %s DB table reset\n",
			getCurrentDate().c_str(),
			getCurrentTime().c_str());
	}
	catch (sql::SQLException e) {
		cout << e.what() << endl;
		return int(PROCESS_RESULT::DB_CREATE_ERROR);
	}
}