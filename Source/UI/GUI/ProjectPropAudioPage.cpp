/*
 * Copyright (C) 2006-2007 Christian Kindahl, christian dot kindahl at gmail dot com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "stdafx.h"
#include "ProjectPropAudioPage.h"
#include "Settings.h"
#include "StringTable.h"
#include "ProjectManager.h"
#include "EditTrackDlg.h"
#include "LangUtil.h"
#include "TransUtil.h"

CProjectPropAudioPage::CProjectPropAudioPage()
{
	// Try to load translated string.
	if (g_LanguageSettings.m_pLNGProcessor != NULL)
	{	
		// Make sure that there is a strings translation section.
		if (g_LanguageSettings.m_pLNGProcessor->EnterSection(_T("strings")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLNGProcessor->GetValuePtr(TITLE_AUDIO,szStrValue))
				SetTitle(szStrValue);
		}
	}

	m_psp.dwFlags |= PSP_HASHELP;
}

CProjectPropAudioPage::~CProjectPropAudioPage()
{
}

bool CProjectPropAudioPage::Translate()
{
	if (g_LanguageSettings.m_pLNGProcessor == NULL)
		return false;

	CLNGProcessor *pLNG = g_LanguageSettings.m_pLNGProcessor;
	
	// Make sure that there is a projectprop translation section.
	if (!pLNG->EnterSection(_T("projectprop")))
		return false;

	// Translate.
	TCHAR *szStrValue;
	int iMaxStaticRight = 0;

	if (pLNG->GetValuePtr(IDC_ALBUMSTATIC,szStrValue))
	{
		SetDlgItemText(IDC_ALBUMSTATIC,szStrValue);

		// Update the static width if necessary.
		int iStaticRight = UpdateStaticWidth(m_hWnd,IDC_ALBUMSTATIC,szStrValue);
		if (iStaticRight > iMaxStaticRight)
			iMaxStaticRight = iStaticRight;
	}
	if (pLNG->GetValuePtr(IDC_ARTISTSTATIC,szStrValue))
	{
		SetDlgItemText(IDC_ARTISTSTATIC,szStrValue);

		// Update the static width if necessary.
		int iStaticRight = UpdateStaticWidth(m_hWnd,IDC_ARTISTSTATIC,szStrValue);
		if (iStaticRight > iMaxStaticRight)
			iMaxStaticRight = iStaticRight;
	}
	if (pLNG->GetValuePtr(IDC_INFOSTATIC,szStrValue))
		SetDlgItemText(IDC_INFOSTATIC,szStrValue);

	// Make sure that the edit/combo controls are not in the way of the statics.
	if (iMaxStaticRight > 75)
	{
		UpdateEditPos(m_hWnd,IDC_ALBUMEDIT,iMaxStaticRight);
		UpdateEditPos(m_hWnd,IDC_ARTISTEDIT,iMaxStaticRight);
	}

	return true;
}

bool CProjectPropAudioPage::OnApply()
{
	GetDlgItemText(IDC_ALBUMEDIT,g_ProjectSettings.m_szAlbumName,159);
	GetDlgItemText(IDC_ARTISTEDIT,g_ProjectSettings.m_szAlbumArtist,159);

	return true;
}

void CProjectPropAudioPage::OnHelp()
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

	ExtractFilePath(szFileName);
	lstrcat(szFileName,lngGetManual());
	lstrcat(szFileName,_T("::/how_to_use/working_with_projects/project_settings.html"));

	HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
}

LRESULT CProjectPropAudioPage::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	// Setup the list view.
	m_ListView = GetDlgItem(IDC_TRACKLIST);
	m_ListView.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);

	m_ListView.InsertColumn(0,lngGetString(COLUMN_TRACK),LVCFMT_LEFT,45,0);
	m_ListView.InsertColumn(1,lngGetString(COLUMN_TITLE),LVCFMT_LEFT,150,1);
	m_ListView.InsertColumn(2,lngGetString(COLUMN_ARTIST),LVCFMT_LEFT,118,2);

	g_ProjectManager.ListAudioTracks(&m_ListView);

	// Load the album information.
	SetDlgItemText(IDC_ALBUMEDIT,g_ProjectSettings.m_szAlbumName);
	SetDlgItemText(IDC_ARTISTEDIT,g_ProjectSettings.m_szAlbumArtist);

	// Translate the window.
	Translate();

	return TRUE;
}

LRESULT CProjectPropAudioPage::OnListDblClick(int iCtrlID,LPNMHDR pNMH,BOOL &bHandled)
{
	if (m_ListView.GetSelectedCount() > 0)
	{
		unsigned int uiTrackIndex = m_ListView.GetSelectedIndex();
		CItemData *pItemData = (CItemData *)m_ListView.GetItemData(uiTrackIndex);

		TCHAR szTitle[64];
		lsnprintf_s(szTitle,64,lngGetString(PROJECTPROP_TRACKPROP),uiTrackIndex + 1);

		CEditTrackDlg  EditTrackDlg(pItemData);
		if (EditTrackDlg.DoModal(::GetActiveWindow(),(LPARAM)szTitle) == IDOK)
		{
			m_ListView.SetItemText(uiTrackIndex,1,pItemData->szTrackTitle);
			m_ListView.SetItemText(uiTrackIndex,2,pItemData->szTrackArtist);
		}
	}

	bHandled = false;
	return 0;
}