﻿
// StripsDlg.h : header file
//

#pragma once

#include "clipper.hpp"
#include "IMMDraw.h"

#include <vector>

// CStripsDlg dialog
class CStripsDlg : public CDialogEx
{
// Construction
public:
	CStripsDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_STRIPS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void add_net();
	afx_msg void draw_everything();
	afx_msg void add_oval();
	afx_msg void add_intersec_oval_and_net();
	CSliderCtrl XPosition;
	CSliderCtrl YPosition;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	CSliderCtrl AngleRotation;
	afx_msg void find_haus_dist();
	CEdit HausdorffDistance;
	CEdit NumOfCoveredBlocks;

	//////////////////////////////////////   Объявления функций   ///////////////////////////////////////
	void add_block(double init_value_x, double init_value_y, double step_value_x, double step_value_y);//

	double count_nearest_distance(std::vector<Paths> net_block, int num_of_block, int num_of_dot);//

	std::vector<Path> do_intersectrion(std::vector<Paths> who_clip, int num_of_path, std::vector<Path> who_clipped);

	void set_slider(CSliderCtrl& slider, int position, int divide_position, CEdit& value);

	afx_msg void do_check_all_positions();
	CEdit CurrXPos;
	CEdit CurrYPos;
	CEdit CurrAnglePos;
	CComboBox Regime;
	CSliderCtrl ScaleOfOval;
	CComboBox NetType;
};
