
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

	std::vector<Path> do_intersectrion(std::vector<Path> who_clip, std::vector<Path> who_clipped);

	std::vector<Path> do_xor(std::vector<Path> who_clip, int num, std::vector<Path> who_clipped);

	std::vector<Path> do_union(std::vector<Paths> who_clip, int num_of_path, std::vector<Path> who_clipped);

	double check_for_empty_ints(std::vector<Path> who_clip, std::vector<Path> who_clipped);

	void CStripsDlg::rotate_and_move_oval();

	void CStripsDlg::rotate_and_move_net(double init_value_x, double init_value_y, double ref_x, double ref_y, double Angle);

	void set_slider(CSliderCtrl& slider, int position, int divide_position, CEdit& value);

	void remove_excess_blocks();

	void set_analysing_points(); // точки, для которых будем считать расстояние хаусдорфа

	void crossed_dots_nearest_blocks(); // информация про тчоки пересечения и ближайшие блоки до них

	void analysis_and_remove_excess_blocks(); // удаляем лишние блоки

	void prepare_points_to_find_Haus(); // очень важная функция. Отбирает только снешние точки кругов массива CoveredNet

	afx_msg void do_check_all_positions();
	CEdit CurrXPos;
	CEdit CurrYPos;
	CEdit CurrAnglePos;
	CComboBox Regime;
	CSliderCtrl ScaleOfOval;
	CComboBox NetType;
	CButton do_ints;
	CButton EnableDrawNet;
	CButton EnableDrawOval;
	CButton EnableDrawInts;
	CButton EnableDrawDots;
	CButton EnableDrawHausDot;
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnBnClickedCheck3();
	afx_msg void OnBnClickedCheck4();
	afx_msg void OnBnClickedCheck5();
	afx_msg void OnBnClickedCheck6();
	CButton EnableOptionDraw;
	afx_msg void OnBnClickedCheck7();
};
