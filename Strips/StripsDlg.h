
// StripsDlg.h : header file
//

#pragma once

#include "clipper.hpp"
#include "IMMDraw.h"

#include <vector>

using namespace std;
using namespace ClipperLib;

void draw_object(vector<Path> object);

typedef struct
{
	double x;
	double y;
} doublePoint;

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
	afx_msg void add_figure();
	afx_msg void add_intersec_figure_and_net();
	CSliderCtrl XPosition;
	CSliderCtrl YPosition;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	CSliderCtrl AngleRotation;
	afx_msg void find_haus_dist();
	CEdit HausdorffDistance;
	CEdit NumOfCoveredBlocks;

	//////////////////////////////////////   Объявления функций   ///////////////////////////////////////
	void add_block(double init_value_x, double init_value_y, double step_value_x, double step_value_y);

	vector<Path> CStripsDlg::do_clip_action(vector<Path> who_clip, vector<Path> who_clipped, ClipType clip_action, int num_of_who_clip = 0, int num_of_who_clipped = 0);

	void CStripsDlg::rotate_and_move_figure();

	void CStripsDlg::rotate_and_move_net(double init_value_x, double init_value_y, double ref_x, double ref_y, double Angle);

	void CStripsDlg::set_editcontrol(double value, CEdit& window);

	void remove_excess_blocks();

	void set_analysing_points(); // точки, для которых будем считать расстояние хаусдорфа

	void crossed_dots_nearest_blocks(); // информация про тчоки пересечения и ближайшие блоки до них

	void analysis_and_remove_excess_blocks(); // удаляем лишние блоки

	void prepare_points_to_find_Haus(); // очень важная функция. Отбирает только снешние точки кругов массива CoveredNet

	void create_oval_cassini();

	void create_square();

	void create_triangle();

	void create_eight();

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
	afx_msg void OnBnClickedCheck6();
	CComboBox FigureType;
	CButton CheckRemoveExcess;
	afx_msg void OnBnClickedCheck9();
	CSliderCtrl NetXRefSlider;
	CSliderCtrl NetYRefSlider;
	CSliderCtrl NetAngleSlider;
	CButton CheckRotateNet;
	afx_msg void OnBnClickedCheck10();
	CSliderCtrl ScaleDrawingSlider;
	afx_msg void OnBnClickedCheck7();
};
