/*
 * InfraRecorder - CD/DVD burning software
 * Copyright (C) 2006-2012 Christian Kindahl
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdafx.hh"
#include "lng_analyzer.hh"

CLngAnalyzer::CLngAnalyzer(const TCHAR *szFullPath) : CLngProcessor(szFullPath)
{
}

CLngAnalyzer::~CLngAnalyzer()
{
}

unsigned int CLngAnalyzer::GetNumValues()
{
    unsigned int uiResult = 0;

    for (unsigned int i = 0; i < m_pSections.size(); i++)
        uiResult += (unsigned int)m_pSections[i]->m_Values.size();

    return uiResult;
}

unsigned int CLngAnalyzer::GetNumSections()
{
    return (unsigned int)m_pSections.size();
}

CLngSection *CLngAnalyzer::GetSection(unsigned int uiIndex)
{
    if (uiIndex > m_pSections.size())
        return NULL;

    return m_pSections[uiIndex];
}

CLngSection *CLngAnalyzer::GetSection(const TCHAR *szName)
{
    for (unsigned int i = 0; i < m_pSections.size(); i++)
    {
        if (!lstrcmp(m_pSections[i]->m_szName,szName))
            return m_pSections[i];
    }

    return NULL;
}

bool CLngAnalyzer::SectionHasValue(CLngSection *pSection,unsigned long ulName)
{
    for (unsigned int i = 0; i < pSection->m_Values.size(); i++)
    {
        if (pSection->m_Values[i]->ulName == ulName)
            return true;
    }

    return false;
}
