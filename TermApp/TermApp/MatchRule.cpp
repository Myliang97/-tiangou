#include "MatchRule.h"

void MatchRule::AddMacthRuleName(std::wstring &name)
{
	m_ruleSet.insert(name);

}

void MatchRule::AddMacthInfo(std::wstring &info)
{
	m_infoSet.insert(info);
}

std::wstring MatchRule::GetMatchRuleName()
{
	m_ruleName.clear();
	for (auto b = m_ruleSet.begin(); b != m_ruleSet.end(); ++b)
	{
		m_ruleName.append(*b);
		m_ruleName.append(L";");
	}
	return m_ruleName;
}

std::wstring MatchRule::GetMatchInfo()
{
	m_info.clear();
	for (auto b =m_infoSet.begin();b!=m_infoSet.end();++b)
	{
		m_info.append(*b);
		m_info.append(L";");
	}
	return m_info;
}

void MatchRule::AddMacthRule(MatchRule &rule)
{
	std::wstring ruleName = rule.GetMatchRuleName();
	int pos = 0;
	int start = 0;
	while ((pos = ruleName.find(L';',start)) && (pos != std::wstring::npos))
	{
		std::wstring dat = ruleName.substr(start, pos -start);
		AddMacthRuleName(dat);
		start = pos + 1;
	}
	std::wstring info = rule.GetMatchInfo();
	pos = 0;
	start = 0;
	while ((pos = info.find(L';',start)) && (pos !=std::wstring::npos))
	{
		std::wstring dat = info.substr(start, pos -start);
		AddMacthInfo(dat);
		start = pos + 1;
	}
}