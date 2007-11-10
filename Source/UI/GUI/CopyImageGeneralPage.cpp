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
#include "CopyImageGeneralPage.h"
#include "Settings.h"
#include "StringTable.h"
#include "DeviceManager.h"
#include "LangUtil.h"
#include "CtrlMessages.h"
#include "TransUtil.h"
#include "WinVer.h"
#include "VisualStyles.h"

CCopyImageGeneralPage::CCopyImageGeneralPage()
{
	m_uiParentTitleLen = 0;
	m_hRefreshIcon = NULL;
	m_hRefreshImageList = NULL;

	m_szFileName[0] = '\0';

	// Try to load translated string.
	if (g_LanguageSettings.m_pLNGProcessor != NULL)
	{	
		// Make sure that there is a strings translation section.
		if (g_LanguageSettings.m_pLNGProcessor->EnterSection(_T("strings")))
		{
			TCHAR *szStrValue;
			if (g_LanguageSettings.m_pLNGProcessor->GetValuePtr(TITLE_GENERAL,szStrValue))
				SetTitle(szStrValue);
		}
	}

	m_psp.dwFlags |= PSP_HASHELP;
}

CCopyImageGeneralPage::~CCopyImageGeneralPage()
{
	if (m_hRefreshImageList != NULL)
		ImageList_Destroy(m_hRefreshImageList);

	if (m_hRefreshIcon != NULL)
		DestroyIcon(m_hRefreshIcon);
}

bool CCopyImageGeneralPage::Translate()
{
	if (g_LanguageSettings.m_pLNGProcessor == NULL)
		return false;

	CLNGProcessor *pLNG = g_LanguageSettings.m_pLNGProcessor;
	
	// Make sure that there is a copy translation section.
	if (!pLNG->EnterSection(_T("copy")))
		return false;

	// Translate.
	TCHAR *szStrValue;
	int iMaxStaticRight = 0;

	if (pLNG->GetValuePtr(IDC_SOURCESTATIC,szStrValue))
	{
		SetDlgItemText(IDC_SOURCESTATIC,szStrValue);

		// Update the static width if necessary.
		int iStaticRight = UpdateStaticWidth(m_hWnd,IDC_SOURCESTATIC,szStrValue);
		if (iStaticRight > iMaxStaticRight)
			iMaxStaticRight = iStaticRight;
	}
	if (pLNG->GetValuePtr(IDC_IMAGESTATIC,szStrValue))
	{
		SetDlgItemText(IDC_IMAGESTATIC,szStrValue);

		// Update the static width if necessary.
		int iStaticRight = UpdateStaticWidth(m_hWnd,IDC_IMAGESTATIC,szStrValue);
		if (iStaticRight > iMaxStaticRight)
			iMaxStaticRight = iStaticRight;
	}

	// Make sure that the edit/combo controls are not in the way of the statics.
	if (iMaxStaticRight > 75)
	{
		UpdateEditPos(m_hWnd,IDC_SOURCECOMBO,iMaxStaticRight);
		UpdateEditPos(m_hWnd,IDC_IMAGEEDIT,iMaxStaticRight);
	}

	return true;
}

void CCopyImageGeneralPage::InitRefreshButton()
{
	m_hRefreshIcon = (HICON)LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(IDI_REFRESHICON),IMAGE_ICON,/*GetSystemMetrics(SM_CXICON)*/16,/*GetSystemMetrics(SM_CYICON)*/16,LR_DEFAULTCOLOR);

	// In Windows XP there is a bug causing the button to loose it's themed style if
	// assigned an icon. The solution to this is to assign an image list instead.
	if (g_WinVer.m_ulMajorCCVersion >= 6)
	{
		m_hRefreshImageList = ImageList_Create(16,16,ILC_COLOR32,0,1);
		ImageList_AddIcon(m_hRefreshImageList,m_hRefreshIcon);

		BUTTON_IMAGELIST bImageList;
		bImageList.himl = m_hRefreshImageList;
		bImageList.margin.left = 0;
		bImageList.margin.top = 0;
		bImageList.margin.right = 0;
		bImageList.margin.bottom = 0;
		bImageList.uAlign = BUTTON_IMAGELIST_ALIGN_CENTER;
		SendMessage(GetDlgItem(IDC_REFRESHBUTTON),BCM_SETIMAGELIST,0,(LPARAM)&bImageList);
	}
	else
	{
		SendMessage(GetDlgItem(IDC_REFRESHBUTTON),BM_SETIMAGE,IMAGE_ICON,(LPARAM)m_hRefreshIcon);
	}

	// If the application is themed, then adjust the size of the button.
	if (g_VisualStyles.IsThemeActive())
	{
		RECT rcButton;
		::GetWindowRect(GetDlgItem(IDC_REFRESHBUTTON),&rcButton);
		ScreenToClient(&rcButton);
		::SetWindowPos(GetDlgItem(IDC_REFRESHBUTTON),NULL,rcButton.left - 1,rcButton.top - 1,
			rcButton.right - rcButton.left + 2,rcButton.bottom - rcButton.top + 2,0);
	}
}

bool CCopyImageGeneralPage::OnApply()
{
	// For internal use only.
	g_CopyDiscSettings.m_iSource = m_SourceCombo.GetItemData(m_SourceCombo.GetCurSel());

	GetDlgItemText(IDC_IMAGEEDIT,m_szFileName,MAX_PATH - 1);

	return true;
}

void CCopyImageGeneralPage::OnHelp()
{
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL,szFileName,MAX_PATH - 1);

	ExtractFilePath(szFileName);
	lstrcat(szFileName,lngGetManual());
	lstrcat(szFileName,_T("::/how_to_use/copy_data_disc.html"));

	HtmlHelp(m_hWnd,szFileName,HH_DISPLAY_TOC,NULL);
}

LRESULT CCopyImageGeneralPage::OnInitDialog(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	m_SourceCombo = GetDlgItem(IDC_SOURCECOMBO);

	// Set the refresh button icon.
	InitRefreshButton();

	// Source combo box.
	for (unsigned int i = 0; i < g_DeviceManager.GetDeviceCount(); i++)
	{
		// We only want to add cdroms to the list.
		if (!g_DeviceManager.IsDeviceReader(i))
			continue;

		tDeviceInfo *pDeviceInfo = g_DeviceManager.GetDeviceInfo(i);

		TCHAR szDeviceName[128];
		g_DeviceManager.GetDeviceName(pDeviceInfo,szDeviceName);

		m_SourceCombo.AddString(szDeviceName);
		m_SourceCombo.SetItemData(m_SourceCombo.GetCount() - 1,i);
	}

	if (m_SourceCombo.GetCount() == 0)
	{
		m_SourceCombo.AddString(lngGetString(FAILURE_NODEVICES));
		m_SourceCombo.SetItemData(0,0);

		m_SourceCombo.EnableWindow(false);
		::EnableWindow(GetDlgItem(IDC_BROWSEBUTTON),false);
	}

	m_SourceCombo.SetCurSel(0);

	// Disable the OK button.
	::EnableWindow(::GetDlgItem(GetParent(),IDOK),false);

	// Let the parent know which source drive that's selected.
	BOOL bDummy;
	OnSourceChange(NULL,NULL,NULL,bDummy);

	// Translate the window.
	Translate();

	return TRUE;
}

LRESULT CCopyImageGeneralPage::OnDestroy(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	m_CurDevice.Close();
	return TRUE;
}

LRESULT CCopyImageGeneralPage::OnTimer(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL &bHandled)
{
	if (m_CurDevice.IsOpen())
	{
		// Check for media change.
		if (g_Core2.CheckMediaChange(&m_CurDevice))
		{
			GetParent().SendMessage(WM_CHECKMEDIA_BROADCAST,0,
				m_SourceCombo.GetItemData(m_SourceCombo.GetCurSel()));
		}
	}

	return TRUE;
}

LRESULT CCopyImageGeneralPage::OnSourceChange(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	UINT_PTR uiSourceDevIndex = m_SourceCombo.GetItemData(m_SourceCombo.GetCurSel());

	::SendMessage(GetParent(),WM_SETDEVICEINDEX,1,(LPARAM)uiSourceDevIndex);

	tDeviceInfo *pDeviceInfo = g_DeviceManager.GetDeviceInfo(uiSourceDevIndex);

	// Open the new device.
	m_CurDevice.Close();
	if (!m_CurDevice.Open(&pDeviceInfo->Address))
		return 0;

	// Kill any already running timers.
	::KillTimer(m_hWnd,TIMER_ID);
	::SetTimer(m_hWnd,TIMER_ID,TIMER_INTERVAL,NULL);

	// Initialize the drive media.
	GetParent().SendMessage(WM_CHECKMEDIA_BROADCAST,0,
		m_SourceCombo.GetItemData(m_SourceCombo.GetCurSel()));

	bHandled = false;
	return 0;
}

LRESULT CCopyImageGeneralPage::OnRefresh(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	// Initialize the drive media.
	GetParent().SendMessage(WM_CHECKMEDIA_BROADCAST,0,
		m_SourceCombo.GetItemData(m_SourceCombo.GetCurSel()));

	return 0;
}

LRESULT CCopyImageGeneralPage::OnBrowse(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL &bHandled)
{
	WTL::CFileDialog FileDialog(false,_T("iso"),_T("Untitled"),OFN_EXPLORER | OFN_OVERWRITEPROMPT,
		_T("Disc Images (*.iso)\0*.iso\0\0"),m_hWnd);
	
	if (FileDialog.DoModal() == IDOK)
	{
		SetDlgItemText(IDC_IMAGEEDIT,FileDialog.m_szFileName);
		::EnableWindow(::GetDlgItem(GetParent(),IDOK),true);
	}

	return 0;
}

TCHAR *CCopyImageGeneralPage::GetFileName()
{
	return m_szFileName;
}