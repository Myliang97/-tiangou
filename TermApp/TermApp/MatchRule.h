#pragma once
#include<string>
#include<set>
class MatchRule
{
public:
	void AddMacthRuleName(std::wstring &name);
	void AddMacthInfo(std::wstring &info);
	void AddMacthRule(MatchRule &rule);
	std::wstring GetMatchRuleName();
	std::wstring GetMatchInfo();
private:
	std::set<std::wstring> m_ruleSet;
	std::set<std::wstring> m_infoSet;
	std::wstring m_ruleName; //������
	std::wstring m_info;  //ƥ������򡢹ؼ���
};