#include "StdAfx.h"
#include "GlobalFunc.h"
#include "MyStruct.h"
#include <map>
#include <list>
#include <string>
#include "Message.h"
#include "MessageList.h"

using namespace std;

extern map<string, InstrumentInfo> InstrumentsSubscribed;
extern list<string> InstrumentsSubscribedList;
extern MapViewType* gMapView;
extern MessageList LogMessageList;
extern HANDLE logSemaphore;
extern char CTPTradingDay[];

GlobalFunc::GlobalFunc(void)
{
}

GlobalFunc::~GlobalFunc(void)
{
}

void GlobalFunc::InitInstrumentSubList()
{
	InstrumentsSubscribed.clear();
	InstrumentsSubscribedList.clear();
	if (gMapView != NULL) {
		for (int i = 0; i < gMapView->instnum; i++) {
			string inst(gMapView->instarray[i]);
			if (inst.find("NYMEX CL") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "NYMEX");
				strcpy(mInstRec.CommodityNo, "CL");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 0.01;
				mInstRec.Multiplier = 1;//外盘缺省为1,未查找
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("COMEX GC") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "COMEX");
				strcpy(mInstRec.CommodityNo, "GC");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 0.1;
				mInstRec.Multiplier = 1;//外盘缺省为1,未查找
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("COMEX SI") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "COMEX");
				strcpy(mInstRec.CommodityNo, "SI");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 0.005;
				mInstRec.Multiplier = 1;//外盘缺省为1,未查找
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("IPE BRN") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "IPE");
				strcpy(mInstRec.CommodityNo, "BRN");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 0.01;
				mInstRec.Multiplier = 1;//外盘缺省为1,未查找
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("CME EC") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "CME");
				strcpy(mInstRec.CommodityNo, "EC");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 0.00005;
				mInstRec.Multiplier = 1;//外盘缺省为1,未查找
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("SGX CN") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "SGX");
				strcpy(mInstRec.CommodityNo, "CN");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 2.5;
				mInstRec.Multiplier = 1;//外盘缺省为1,未查找
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("NYBOT DX") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "NYBOT");
				strcpy(mInstRec.CommodityNo, "DX");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 0.005;
				mInstRec.Multiplier = 1;//外盘缺省为1,未查找
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("CME RMB") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "CME");
				strcpy(mInstRec.CommodityNo, "RMB");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 0.00001;
				mInstRec.Multiplier = 1;//外盘缺省为1,未查找
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("HKFE CUS") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "HKFE");
				strcpy(mInstRec.CommodityNo, "CUS");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 0.0001;
				mInstRec.Multiplier = 1;//外盘缺省为1,未查找
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("SGX UC") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "SGX");
				strcpy(mInstRec.CommodityNo, "UC");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 0.0001;
				mInstRec.Multiplier = 1;//外盘缺省为1,未查找
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("LME CAD") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "LME");
				strcpy(mInstRec.CommodityNo, "CAD");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 0.5;
				mInstRec.Multiplier = 1;//外盘缺省为1,未查找
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("LME AHD") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "LME");
				strcpy(mInstRec.CommodityNo, "AHD");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 0.5;
				mInstRec.Multiplier = 1;//外盘缺省为1,未查找
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("LME ZSD") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "LME");
				strcpy(mInstRec.CommodityNo, "ZSD");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 0.5;
				mInstRec.Multiplier = 1;//外盘缺省为1,未查找
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("SHFE rb") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "SHFE");
				strcpy(mInstRec.CommodityNo, "rb");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 1.0;
				mInstRec.Multiplier = 10;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("SGE Au") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "SGE");
				strcpy(mInstRec.CommodityNo, "Au");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 0.01;
				mInstRec.Multiplier = 1000;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("SGE Ag") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "SGE");
				strcpy(mInstRec.CommodityNo, "Ag");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 1.0;
				mInstRec.Multiplier = 1;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("CFFEX IF") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "CFFEX");
				strcpy(mInstRec.CommodityNo, "IF");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 0.2;
				mInstRec.Multiplier = 300;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("CFFEX IH") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "CFFEX");
				strcpy(mInstRec.CommodityNo, "IH");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 0.2;
				mInstRec.Multiplier = 300;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("CFFEX IC") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "CFFEX");
				strcpy(mInstRec.CommodityNo, "IC");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 0.2;
				mInstRec.Multiplier = 200;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("CFFEX T") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "CFFEX");
				strcpy(mInstRec.CommodityNo, "T");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 0.005;
				mInstRec.Multiplier = 10000;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("SHFE ag") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "SHFE");
				strcpy(mInstRec.CommodityNo, "ag");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 1.0;
				mInstRec.Multiplier = 15;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("SHFE au") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "SHFE");
				strcpy(mInstRec.CommodityNo, "au");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 0.05;
				mInstRec.Multiplier = 1000;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("SHFE cu") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "SHFE");
				strcpy(mInstRec.CommodityNo, "cu");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 10.0;
				mInstRec.Multiplier = 5;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("SHFE ni") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "SHFE");
				strcpy(mInstRec.CommodityNo, "ni");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 10.0;
				mInstRec.Multiplier = 1;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("SHFE al") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "SHFE");
				strcpy(mInstRec.CommodityNo, "al");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 5.0;
				mInstRec.Multiplier = 5;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("SHFE zn") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "SHFE");
				strcpy(mInstRec.CommodityNo, "zn");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 5.0;
				mInstRec.Multiplier = 5;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("SHFE ru") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "SHFE");
				strcpy(mInstRec.CommodityNo, "ru");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 5.0;
				mInstRec.Multiplier = 10;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("SHFE rb") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "SHFE");
				strcpy(mInstRec.CommodityNo, "rb");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 1.0;
				mInstRec.Multiplier = 10;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("SHFE bu") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "SHFE");
				strcpy(mInstRec.CommodityNo, "bu");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 2.0;
				mInstRec.Multiplier = 10;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("DCE y") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "DCE");
				strcpy(mInstRec.CommodityNo, "y");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 2.0;
				mInstRec.Multiplier = 10;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("DCE p") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "DCE");
				strcpy(mInstRec.CommodityNo, "p");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 2.0;
				mInstRec.Multiplier = 10;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("DCE m") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "DCE");
				strcpy(mInstRec.CommodityNo, "m");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 1.0;
				mInstRec.Multiplier = 10;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("DCE jd") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "DCE");
				strcpy(mInstRec.CommodityNo, "jd");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 1.0;
				mInstRec.Multiplier = 10;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("DCE jm") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "DCE");
				strcpy(mInstRec.CommodityNo, "jm");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 0.5;
				mInstRec.Multiplier = 60;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("DCE j") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "DCE");
				strcpy(mInstRec.CommodityNo, "j");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 0.5;
				mInstRec.Multiplier = 100;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}

			else if (inst.find("DCE i") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "DCE");
				strcpy(mInstRec.CommodityNo, "i");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 0.5;
				mInstRec.Multiplier = 100;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("DCE pp") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "DCE");
				strcpy(mInstRec.CommodityNo, "pp");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 1.0;
				mInstRec.Multiplier = 5;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("DCE l") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "DCE");
				strcpy(mInstRec.CommodityNo, "l");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 5.0;
				mInstRec.Multiplier = 5;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("DCE c") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "DCE");
				strcpy(mInstRec.CommodityNo, "c");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 1.0;
				mInstRec.Multiplier = 10;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("CZCE SR") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "CZCE");
				strcpy(mInstRec.CommodityNo, "SR");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 1.0;
				mInstRec.Multiplier = 10;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("DCE v") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "DCE");
				strcpy(mInstRec.CommodityNo, "v");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 5.0;
				mInstRec.Multiplier = 5;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("CZCE MA") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "CZCE");
				strcpy(mInstRec.CommodityNo, "MA");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 1.0;
				mInstRec.Multiplier = 10;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("CZCE CF") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "CZCE");
				strcpy(mInstRec.CommodityNo, "CF");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 5.0;
				mInstRec.Multiplier = 5;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("CZCE TA") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "CZCE");
				strcpy(mInstRec.CommodityNo, "TA");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 2.0;
				mInstRec.Multiplier = 5;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("CZCE FG") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "CZCE");
				strcpy(mInstRec.CommodityNo, "FG");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 1.0;
				mInstRec.Multiplier = 20;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("CZCE SF") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "CZCE");
				strcpy(mInstRec.CommodityNo, "SF");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 2.0;
				mInstRec.Multiplier = 5;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("CZCE SM") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "CZCE");
				strcpy(mInstRec.CommodityNo, "SM");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 2.0;
				mInstRec.Multiplier = 5;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("CZCE OI") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "CZCE");
				strcpy(mInstRec.CommodityNo, "OI");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 1.0;
				mInstRec.Multiplier = 10;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}

			else if (inst.find("CZCE RM") != std::string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "CZCE");
				strcpy(mInstRec.CommodityNo, "RM");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 1.0;
				mInstRec.Multiplier = 10;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
			else if (inst.find("INE sc") != string::npos) {
				InstrumentInfo mInstRec;
				strcpy(mInstRec.ExchangeID, "INE");
				strcpy(mInstRec.CommodityNo, "sc");
				string strInstID = inst.substr(inst.find_last_of(" ") + 1, inst.length() - inst.find_last_of(" "));
				strcpy(mInstRec.InstrumentID, strInstID.c_str());
				mInstRec.OneTick = 0.1;
				mInstRec.Multiplier = 1000;
				InstrumentsSubscribed.insert(std::pair<string, InstrumentInfo>(inst, mInstRec));
				InstrumentsSubscribedList.push_back(inst);
			}
		}
	}
}

void GlobalFunc::ConvertCStringToCharArray(CString csSource, char* rtnCharArray)
{
	int cslen = WideCharToMultiByte(CP_ACP, 0, csSource, csSource.GetLength(), NULL, 0, NULL, NULL);
	char* carray = new char[cslen + 1];
	WideCharToMultiByte(CP_ACP, 0, csSource, csSource.GetLength(), carray, cslen, NULL, NULL);
	carray[cslen] = '\0';
	strcpy(rtnCharArray, carray);
	delete carray;
}

void GlobalFunc::WriteMsgToLogList(char logline[200])
{
	Message logMsg;
	logMsg.type = STRATEGY_LOG;
	logMsg.AddData(logline, 0, sizeof(char) * 200);
	LogMessageList.AddTail(logMsg);
	ReleaseSemaphore(logSemaphore, 1, NULL);
}

int GlobalFunc::Split(CString source, CString ch, CStringArray& strarr)
{
	/*---------------------------------------------------------

	* 函数介绍： 从原字符串里按照指定的分隔字符串进行分割,将分隔的结果存放到字符串数组里

	* 输入参数：
	source -- 原字符串
	ch -- 指定的分隔字符串
	strarr -- 外部引用的一个字符串数组

	* 输出参数：

	* 返回值 ：总共分隔了多少段字符串.

	-----------------------------------------------------------*/
	CString TmpStr;
	strarr.RemoveAll();
	if (source.IsEmpty() || ch.IsEmpty())
		return 0;
	int len = ch.GetLength();
	int findi = 0;
	int findn = 0;
	int sum = 0;

	findn = source.Find(ch, findi);
	if (findn != -1)
	{
		TmpStr = source.Mid(0, findn);
		//TmpStr.Trim();
		strarr.Add(TmpStr);
		findi = findn + len;
		sum++;
	}
	else
	{
		//source.Trim();
		strarr.Add(source);
		sum++;
		return sum;
	}
	while (findn != -1)//有发现
	{
		findn = source.Find(ch, findi);
		if (findn != -1)
		{
			TmpStr = source.Mid(findi, findn - findi);
			//TmpStr.Trim();//去除头尾空格
			strarr.Add(TmpStr);

			findi = findn + len;
			sum++;
		}
		else
		{
			TmpStr = source.Mid(findi, source.GetLength() - findi);
			//TmpStr.Trim();
			strarr.Add(TmpStr);
			sum++;
		}
	}

	return sum;
}

char* GlobalFunc::getModuleFilePath(char* path)
{
	if (!path) return NULL;
	CString strPathTradeFile;
	::GetModuleFileName(NULL, strPathTradeFile.GetBuffer(MAX_PATH), MAX_PATH);
	strPathTradeFile.ReleaseBuffer();
	strPathTradeFile = strPathTradeFile.Left(strPathTradeFile.ReverseFind(_T('\\')));

	int len = WideCharToMultiByte(CP_ACP, 0, strPathTradeFile, strPathTradeFile.GetLength(), NULL, 0, NULL, NULL);
	char* c_str_filename = new char[len + 1];
	WideCharToMultiByte(CP_ACP, 0, strPathTradeFile, strPathTradeFile.GetLength(), c_str_filename, len, NULL, NULL);
	c_str_filename[len] = '\0';
	strcpy(path, c_str_filename);
	delete c_str_filename;
	return path;
}

void GlobalFunc::getUDPSendLog(TradeLogType* pTrade, SendLogType* pSend) {
	strcpy(pSend->AccountID, pTrade->AccountID);
	strcpy(pSend->StrategyID, pTrade->StrategyID);
	strcpy(pSend->InstanceName, pTrade->InstanceName);
	strcpy(pSend->CodeName, pTrade->CodeName);
	strcpy(pSend->MatchNo, pTrade->MatchNo);
	//strcpy(pSend->tradingday,pTrade->tradingday);
	strcpy(pSend->tradingday, CTPTradingDay);
	strcpy(pSend->tradingtime, pTrade->tradingtime);
	pSend->tradeprice = pTrade->tradeprice;
	pSend->submitprice = pTrade->submitprice;
	pSend->qty = pTrade->qty;
	pSend->fee = pTrade->fee;
	pSend->openorclose = pTrade->openorclose;
	sprintf(pSend->openid, "%d", pTrade->openid);
	sprintf(pSend->closeid, "%d", pTrade->closeid);
}