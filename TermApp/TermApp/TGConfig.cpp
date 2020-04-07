#include "TGConfig.h"
#include<log/DebugString.hpp>
#include"TermApp.h"
#include<global.h>
#include<stringLib/strLib.hpp>

TGConfig*TGConfig::m_config = NULL;

BOOL TGConfig::UpdateConfig(const char *xmlData/* NULL */)
{
	XmlReader reader;
	if (xmlData)
	{
		if (!reader.LoadXml(xmlData))
		{
			TermApp::Instance()->GetLogger()->Trace(INFO_FORMAT, "load xml data error %d",GetLastError());
			return FALSE;
		}
	}
	else
	{
		TermApp *app = TermApp::Instance();
		CHAR confPath[MAX_PATH];
		sprintf(confPath, "%s\\%s",app->GetTermInfo()->m_tgDir.c_str(), TG_CONFIG_PATH);
		if (!reader.LoadFile(confPath))
		{
			TermApp::Instance()->GetLogger()->Trace(INFO_FORMAT, "load xml file error %d",GetLastError());
			return FALSE;
		}
	}
	return LoadConf(reader);
}

BOOL TGConfig::LoadConf(XmlReader &reader)
{
	try
	{
		const char *head = reader.SelectNode("head")->GetText();
		if (0 != strcmp(head, "0"))
		{
			return FALSE;
		}
		 const char *serverIp = reader.SelectNode("server.ip")->GetText();
		 wchar_t *wzserverIp = StrLib::Utf8ToUnicode(serverIp);
		 if (wzserverIp)
		 {
			 m_configData->m_server.m_ip = wzserverIp;
			 delete[]wzserverIp;
		 }
		int port = atoi(reader.SelectNode("server.port")->GetText());
		if (port <= 0)
		{
			port = 80;	
		}
		m_configData->m_server.m_port = port;
		TGDebug::debugW(DEBUG_STRING, L"server ip:%s,port:%d", m_configData->m_server.m_ip.c_str(), port);

		TiXmlElement *policy = reader.SelectNode("policy");
		int num = reader.GetChildNodeCount(policy);
		for (int i = 0; i < num; ++i)
		{
			ScanFileRule file_rule;
			TiXmlElement *rule = reader.SelectIndexChildNode(policy, i);
			const char *name = rule->Attribute("name");
			wchar_t *wname = StrLib::Utf8ToUnicode(name);
			if (wname) {
				file_rule.m_ruleName = wname;
				TGDebug::debugW(DEBUG_STRING, L"name:%s", wname);
				delete[]wname;
			}
			TiXmlElement *types = reader.SelectChildNode(rule, "types");
			int len = reader.GetChildNodeCount(types);
			for (int j = 0; j < len; ++j)
			{
				TiXmlElement *type = reader.SelectIndexChildNode(types, j);
				wchar_t *wtype = StrLib::Utf8ToUnicode(type->GetText());
				if (wtype)
				{
					TGDebug::debugW(DEBUG_STRING, L"type:%s", wtype);
					file_rule.m_types.push_back(wtype);
					delete[]wtype;
				}
			}
			TiXmlElement *keywords = reader.SelectChildNode(rule, "keywords");
			len = reader.GetChildNodeCount(keywords);
			for (int j=0;j<len;++j)
			{
				TiXmlElement *keyword = reader.SelectIndexChildNode(keywords, j);
				wchar_t *wtext = StrLib::Utf8ToUnicode(keyword->GetText());
				KeyWordAttribute attr;
				if (wtext)
				{
					TGDebug::debugW(DEBUG_STRING, L"keyword:%s", wtext);
					attr.m_keyword = wtext;
					delete[]wtext;
				}
				const char *strnum = keyword->Attribute("num");
				attr.num = atoi(strnum);
				file_rule.m_keywords.push_back(attr);
			}

			TiXmlElement *regexs = reader.SelectChildNode(rule, "regexs");
			len = reader.GetChildNodeCount(regexs);
			for (int j = 0; j < len; ++j)
			{
				TiXmlElement *regex = reader.SelectIndexChildNode(regexs, j);
				wchar_t *wtext = StrLib::Utf8ToUnicode(regex->GetText());
				RegexAttribute attr;
				if (wtext)
				{
					TGDebug::debugW(DEBUG_STRING, L"regex:%s", wtext);
					attr.m_regex = wtext;
					delete[]wtext;
				}
				const char *strnum = regex->Attribute("num");
				attr.num = atoi(strnum);
				file_rule.m_regexs.push_back(attr);
			}
			m_configData->m_scanFileRuleList.push_back(file_rule);
		}

		TiXmlElement *oper = reader.SelectNode("operator");
		if (oper)
		{
			FileOperator fileOper;
			fileOper.m_encrypt = atoi(oper->Attribute("encrypt"));
			fileOper.oper = atoi(oper->Attribute("oper"));
			m_configData->m_oper = fileOper;
		}

		TiXmlElement *uploadFile = reader.SelectNode("uploadfile");
		if (uploadFile)
		{
			UploadFile ulFile;
			const char *sizeStr = uploadFile->Attribute("size");
			int sizeint = atoi(sizeStr);
			int suint = 0;
			const char *strUnit = uploadFile->Attribute("unit"); //unit:0:b 1:kb 2:mb
			suint = atoi(strUnit);
			if (suint == 0)
			{
				ulFile.m_size = sizeint;
			}
			else if (suint == 1)
			{
				ulFile.m_size = sizeint * 1024;
			}
			else
			{
				ulFile.m_size = sizeint * 1024 * 1024;
			}
			ulFile.m_cmp =atoi(uploadFile->Attribute("cmp"));
			ulFile.m_type = atoi(uploadFile->Attribute("type"));
			m_configData->m_uploadFileRule = ulFile;
		}
	}
	catch (...)
	{
		return FALSE;
	}
	return TRUE;
}
TGConfig *TGConfig::Instance()
{
	if (m_config == NULL)
	{
		m_config = new TGConfig;
	}
	return m_config;
}
TGConfig::TGConfig()
{
	m_configData = new ConfigData;
}


TGConfig::~TGConfig()
{
	if (m_configData)
	{
		delete m_configData;
	}
}
