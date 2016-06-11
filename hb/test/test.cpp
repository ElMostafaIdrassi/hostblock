// Standard input/output stream library (cin, cout, cerr, clog)
#include <iostream>
// Time (clock_t, clock())
#include <time.h>
// Standard map library
#include <map>
// Standard vector library
#include <vector>
// Syslog
namespace csyslog{
	#include <syslog.h>
}
// Logger
#include "../src/logger.h"
// Iptables
#include "../src/iptables.h"
// Config
#include "../src/config.h"
// Data
#include "../src/data.h"

int main(int argc, char *argv[])
{
	clock_t start = clock();
	clock_t end;

	time_t currentTime;
	time(&currentTime);

	bool testSyslog = false;
	bool testIptables = true;
	bool testConfig = false;
	bool testData = true;
	bool removeTempData = true;

	try{
		// Syslog
		std::cout << "Creating Logger object..." << std::endl;
		hb::Logger log = hb::Logger(LOG_USER);// LOG_USER is facility, priority is changed by setlevel
		if (testSyslog){
			log.setLevel(LOG_ERR);
			std::cout << "Writting to syslog with level LOG_ERR..." << std::endl;
			log.info("Syslog test - level: LOG_ERR msg type: info");
			log.warning("Syslog test - level: LOG_ERR msg type: warning");
			log.error("Syslog test - level: LOG_ERR msg type: error");
			log.debug("Syslog test - level: LOG_ERR msg type: debug");
			log.setLevel(LOG_WARNING);
			std::cout << "Writting to syslog with level LOG_WARNING..." << std::endl;
			log.info("Syslog test - level: LOG_WARNING msg type: info");
			log.warning("Syslog test - level: LOG_WARNING msg type: warning");
			log.error("Syslog test - level: LOG_WARNING msg type: error");
			log.debug("Syslog test - level: LOG_WARNING msg type: debug");
			log.setLevel(LOG_NOTICE);
			std::cout << "Writting to syslog with level LOG_NOTICE..." << std::endl;
			log.info("Syslog test - level: LOG_NOTICE msg type: info");
			log.warning("Syslog test - level: LOG_NOTICE msg type: warning");
			log.error("Syslog test - level: LOG_NOTICE msg type: error");
			log.debug("Syslog test - level: LOG_NOTICE msg type: debug");
			log.setLevel(LOG_INFO);
			std::cout << "Writting to syslog with level LOG_INFO..." << std::endl;
			log.info("Syslog test - level: LOG_INFO msg type: info");
			log.warning("Syslog test - level: LOG_INFO msg type: warning");
			log.error("Syslog test - level: LOG_INFO msg type: error");
			log.debug("Syslog test - level: LOG_INFO msg type: debug");
			log.setLevel(LOG_DEBUG);
			std::cout << "Writting to syslog with level LOG_DEBUG..." << std::endl;
			log.info("Syslog test - level: LOG_DEBUG msg type: info");
			log.warning("Syslog test - level: LOG_DEBUG msg type: warning");
			log.error("Syslog test - level: LOG_DEBUG msg type: error");
			log.debug("Syslog test - level: LOG_DEBUG msg type: debug");
		}
		log.setLevel(LOG_DEBUG);

		// iptables
		std::cout << "Creating Iptables object..." << std::endl;
		hb::Iptables iptbl = hb::Iptables();
		if (testIptables){
			std::map<unsigned int, std::string> rules;
			std::map<unsigned int, std::string>::iterator ruleIt;
			std::cout << "iptable rules (INPUT):" << std::endl;
			rules = iptbl.listRules("INPUT");
			for(ruleIt=rules.begin(); ruleIt!=rules.end(); ++ruleIt){
				std::cout << "Rule: " << ruleIt->second << std::endl;
			}
			std::cout << "Adding rule to drop all connections from 10.10.10.10..." << std::endl;
			if(iptbl.append("INPUT","-s 10.10.10.10 -j DROP") == false){
				std::cerr << "Failed to add rule for address 10.10.10.10" << std::endl;
			}
			std::cout << "iptable rules (INPUT):" << std::endl;
			rules = iptbl.listRules("INPUT");
			for(ruleIt=rules.begin(); ruleIt!=rules.end(); ++ruleIt){
				std::cout << "Rule: " << ruleIt->second << std::endl;
			}
			std::cout << "Removing rule for address 10.10.10.10..." << std::endl;
			if(iptbl.remove("INPUT","-s 10.10.10.10 -j DROP") == false){
				std::cerr << "Failed to remove rule for address 10.10.10.10" << std::endl;
			}
			std::cout << "iptable rules (INPUT):" << std::endl;
			rules = iptbl.listRules("INPUT");
			for(ruleIt=rules.begin(); ruleIt!=rules.end(); ++ruleIt){
				std::cout << "Rule: " << ruleIt->second << std::endl;
			}
		}

		// Config
		std::cout << "Creating Config object..." << std::endl;
		hb::Config cfg = hb::Config(&log, "config/hostblock.conf");
		std::cout << "Loading configuration file..." << std::endl;
		if (!cfg.load()){
			std::cerr << "Failed to load configuration!" << std::endl;
		}
		if (testConfig){
			std::cout << "Printing configuration to stdout..." << std::endl;
			cfg.print();
		}

		// Data
		std::cout << "Creating Data object..." << std::endl;
		hb::Data data = hb::Data(&log, &cfg, &iptbl);
		cfg.dataFilePath = "hb/test/test_data";
		std::cout << "Loading data..." << std::endl;
		if (!data.loadData()) {
			std::cerr << "Failed to load data!" << std::endl;
		}
		if (testData){
			// Create new data file
			std::cout << "Creating new data file..." << std::endl;
			cfg.dataFilePath = "test_data_tmp";
			if (!data.saveData()) {
				std::cerr << "Failed to save data to datafile!" << std::endl;
			}
			std::cout << "suspiciousAddresses.size = " << std::to_string(data.suspiciousAddresses.size()) << std::endl;
			// Append single record to datafile
			std::cout << "Adding 10.10.10.15 to data file..." << std::endl;
			hb::SuspiciosAddressType rec;
			rec.lastActivity = (unsigned long long int)currentTime;
			rec.activityScore = 99;
			rec.activityCount = 3;
			rec.refusedCount = 2;
			rec.whitelisted = false;
			rec.blacklisted = true;
			data.suspiciousAddresses.insert(std::pair<std::string,hb::SuspiciosAddressType>("10.10.10.15",rec));
			if (!data.addAddress("10.10.10.15")) {
				std::cerr << "Failed to add new record to datafile!" << std::endl;
			}
			// Update single record in datafile
			std::cout << "Updating 10.10.10.15 in data file..." << std::endl;
			data.suspiciousAddresses["10.10.10.15"].activityScore += 10;
			data.suspiciousAddresses["10.10.10.15"].activityCount++;
			data.suspiciousAddresses["10.10.10.15"].lastActivity = (unsigned long long int)currentTime;
			if (!data.updateAddress("10.10.10.15")) {
				std::cerr << "Failed to update record in datafile!" << std::endl;
			}
			std::cout << "Updating 10.10.10.14 in data file..." << std::endl;
			data.suspiciousAddresses["10.10.10.14"].lastActivity = (unsigned long long int)currentTime;
			if (!data.updateAddress("10.10.10.14")) {
				std::cerr << "Failed to update record in datafile!" << std::endl;
			}
			// Remove single record from datafile
			std::cout << "Removing 10.10.10.13 from data file..." << std::endl;
			if (!data.removeAddress("10.10.10.13")) {
				std::cerr << "Failed to remove record from datafile!" << std::endl;
			}
			// Add new log file to datafile
			std::cout << "Adding /var/log/messages to data file..." << std::endl;
			std::vector<hb::LogGroup>::iterator itlg;
			for (itlg = cfg.logGroups.begin(); itlg != cfg.logGroups.end(); ++itlg) {
				if (itlg->name == "OpenSSH") {
					hb::LogFile logFile;
					logFile.path = "/var/log/messages";
					logFile.bookmark = 800;
					logFile.size = 800;
					itlg->logFiles.push_back(logFile);
					break;
				}
			}
			if (!data.addFile("/var/log/messages")) {
				std::cerr << "Failed to add new record to datafile!" << std::endl;
			}
			// Update bookmark and size for log file in datafile
			std::cout << "Updating /var/log/messages in data file..." << std::endl;
			// std::vector<hb::LogGroup>::iterator itlg;
			std::vector<hb::LogFile>::iterator itlf;
			for (itlg = cfg.logGroups.begin(); itlg != cfg.logGroups.end(); ++itlg) {
				for (itlf = itlg->logFiles.begin(); itlf != itlg->logFiles.end(); ++itlf) {
					if (itlf->path == "/var/log/messages") {
						itlf->bookmark += 100;
						itlf->size += 100;
						break;
					}
				}
			}
			if (!data.updateFile("/var/log/messages")) {
				std::cerr << "Failed to update record in datafile!" << std::endl;
			}
			// Remove log file record from datafile
			std::cout << "Removing /var/log/auth.log from data file..." << std::endl;
			if (!data.removeFile("/var/log/auth.log")) {
				std::cerr << "Failed to remove record from datafile!" << std::endl;
			}
			// Load data file
			std::cout << "Loading data from new datafile..." << std::endl;
			if (!data.loadData()) {
				std::cerr << "Failed to load data!" << std::endl;
			}
			std::cout << "suspiciousAddresses.size = " << std::to_string(data.suspiciousAddresses.size()) << std::endl;
			// Remove newly created data file
			if (removeTempData && std::remove(cfg.dataFilePath.c_str()) != 0) {
				std::cerr << "Failed to remove temporary data file!" << std::endl;
			}
			// Compare data with iptables
			if (testIptables) {
				std::cout << "Comparing data with iptables..." << std::endl;
				if (!data.checkIptables()) {
					std::cerr << "Failed to compare data with iptables!" << std::endl;
				}
			}
		}

	} catch (std::exception& e){
		std::cerr << e.what() << std::endl;
		end = clock();
		std::cout << "Exec time: " << (double)(end - start)/CLOCKS_PER_SEC << " sec" << std::endl;
		return 1;
	}

	end = clock();
	std::cout << "Exec time: " << (double)(end - start)/CLOCKS_PER_SEC << " sec" << std::endl;
}
