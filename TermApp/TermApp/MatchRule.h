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
	std::wstring m_ruleName; //规则名
	std::wstring m_info;  //匹配的正则、关键词
};