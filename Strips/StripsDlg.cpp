// StripsDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "Strips.h"
#include "StripsDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

////////////////////////////////////// Глобальные переменные  ///////////////////////////////////////

double RadiusOfBlock = 0.5; // радиус круглого блока сетки
CRect RedrawArea; // область перерисовки
int scale_helper = 1e6; // коэф. для коннектной работы библиотеки clipper, которая понимает только целые числа (int)
						   // пример: sqrt(2)*1e6 = 1.414213*1e6 = 1414213 - итоговое число в расчете пересечений. 
int window_center_x = 0; // координаты центра области перерисовки
int window_center_y = 0;
int scale_drawing = 20; // коэф. для более крупной отрисовки

int SliderLimit = 5; // диапазон для слайдеров X и Y
int AngleSliderCoef = 1; // коэф. для слайдера углов. Если хотим не целые углы, то > 1.

vector<Paths> Net(1); // сетка покрытия
vector<Path> Block(1); // отдельный блок сетки
vector<Paths> CoveredNet(1); // только те элементы сетки, которые покрывают фигуру (овал)
vector<Paths> Figure(1); // изначальный в начале координат. На его основании поворачиваем и двигаем
vector<Paths> RotatedAndMovedFigure(1);
vector<Paths> Intersections(1);

vector<Paths> CrossedDots(1); // точки пересечения блока и фигуры (овала)
vector<IntPoint> CentersOfBlocks; // центры всех блоков сетки
vector<IntPoint> CentersOfCoveredBlocks; // центры блоков, покрывающих фигуру (овал

vector<Path> AnalysingHausDistPoints(1); // все точки, для которых будет посчитано кратчайшее расстояние до фигуры (овала)

double FigureXMax = 0; // граничные значения фигуры (овала)
double FigureYMax = 0;

int SliderCoreff = 1; // коэффициент для настройки диапазона слайдера. 1 - для кругов, 2 - дял квадратов
double BlockArea = 0; // площадь одного блока сетки

double init_value_x_rotated = 0, init_value_y_rotated = 0; //вспомогательные переменные на случай вращения сетки
double HausDist = 0; // указываем расстояние, которое точно меньше минимального

typedef struct
{
	int BlockID; // номер покрывающего блока в массиве CoveredNet
	int DotID; // номер точки пересечения с овалом Каасини
	int NearestID; // номер ближайшего блока в массиве CoveredNet
} InfoOFBlockCrossedOval;

vector<InfoOFBlockCrossedOval> CrossedBlocksInfo; // необходимая информация о блоках сетки, покрывающих фигуру (овал)

// чтобы рисовать объекты типа Clipperlib::vector<Path> их надо превартить в структуры типа POINT
POINT ** StructureForDrawPaths;
POINT * StructureForDrawPath;

CPen blackpen(PS_SOLID, 1, RGB(0, 0, 0));
CPen redpen(PS_SOLID, 1, RGB(255, 0, 0));
CPen bluepen(PS_SOLID, 1, RGB(0, 0, 255));
CPen greenpen(PS_SOLID, 1, RGB(0, 255, 0));

int XHaus = 0; // какая точка соответствует Хаусдорфовому расстоянию
int YHaus = 0;

IntPoint OvalHausPrev; // предварительная точка на овале Кассини, соответствующая расстоянию Хаусдорфа
IntPoint OvalHaus; // точка на овале Кассини, соответствующая расстоянию Хаусдорфа

// площади пересечения элементов сетки и овала Кассини (size = Net[0].size())
vector<double> Areas;
vector<double> CoveredsAreas;

vector<Paths> CheckedDots_new(1);

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CStripsDlg dialog

CStripsDlg::CStripsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_STRIPS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CStripsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER1, XPosition);
	DDX_Control(pDX, IDC_SLIDER2, YPosition);
	DDX_Control(pDX, IDC_SLIDER3, AngleRotation);
	DDX_Control(pDX, IDC_EDIT1, HausdorffDistance);
	DDX_Control(pDX, IDC_EDIT2, NumOfCoveredBlocks);
	DDX_Control(pDX, IDC_EDIT3, CurrXPos);
	DDX_Control(pDX, IDC_EDIT4, CurrYPos);
	DDX_Control(pDX, IDC_EDIT5, CurrAnglePos);
	DDX_Control(pDX, IDC_COMBO1, Regime);
	DDX_Control(pDX, IDC_SLIDER4, ScaleOfOval);
	DDX_Control(pDX, IDC_COMBO2, NetType);
	DDX_Control(pDX, IDC_CHECK2, EnableDrawNet);
	DDX_Control(pDX, IDC_CHECK3, EnableDrawOval);
	DDX_Control(pDX, IDC_CHECK4, EnableDrawInts);
	DDX_Control(pDX, IDC_CHECK6, EnableDrawHausDot);
	DDX_Control(pDX, IDC_COMBO3, FigureType);
	DDX_Control(pDX, IDC_CHECK9, CheckRemoveExcess);
	DDX_Control(pDX, IDC_SLIDER5, NetXRefSlider);
	DDX_Control(pDX, IDC_SLIDER6, NetYRefSlider);
	DDX_Control(pDX, IDC_SLIDER7, NetAngleSlider);
	DDX_Control(pDX, IDC_CHECK10, CheckRotateNet);
	DDX_Control(pDX, IDC_SLIDER8, ScaleDrawingSlider);
	DDX_Control(pDX, IDC_CHECK7, EnableDrawDots);
}

BEGIN_MESSAGE_MAP(CStripsDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CStripsDlg::add_net)
	ON_BN_CLICKED(IDC_BUTTON2, &CStripsDlg::draw_everything)
	ON_BN_CLICKED(IDC_BUTTON3, &CStripsDlg::add_figure)
	ON_BN_CLICKED(IDC_BUTTON4, &CStripsDlg::add_intersec_figure_and_net)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON5, &CStripsDlg::find_haus_dist)
	ON_BN_CLICKED(IDC_BUTTON6, &CStripsDlg::do_check_all_positions)
	ON_BN_CLICKED(IDC_CHECK2, &CStripsDlg::OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK3, &CStripsDlg::OnBnClickedCheck3)
	ON_BN_CLICKED(IDC_CHECK4, &CStripsDlg::OnBnClickedCheck4)
	ON_BN_CLICKED(IDC_CHECK6, &CStripsDlg::OnBnClickedCheck6)
	ON_BN_CLICKED(IDC_CHECK9, &CStripsDlg::OnBnClickedCheck9)
	ON_BN_CLICKED(IDC_CHECK10, &CStripsDlg::OnBnClickedCheck10)
	ON_BN_CLICKED(IDC_CHECK7, &CStripsDlg::OnBnClickedCheck7)
END_MESSAGE_MAP()

// CStripsDlg message handlers

BOOL CStripsDlg::OnInitDialog()
{

	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}

		GetClientRect(&RedrawArea);
		window_center_x = RedrawArea.Width() / 2;
		window_center_y = RedrawArea.Height() / 2;

		ScaleOfOval.SetRange(1, 7, 1);
		ScaleOfOval.SetPos(5);

		RedrawArea.SetRect(window_center_x - 10 * scale_drawing, window_center_y - 10 * scale_drawing, window_center_x + 10 * scale_drawing, window_center_y + 10 * scale_drawing);
		set_drawing_param(window_center_x, window_center_y, scale_drawing, scale_helper);

		XPosition.SetRange(-SliderLimit, SliderLimit, 1);
		XPosition.SetPos(0); //0

		YPosition.SetRange(-SliderLimit, SliderLimit, 1);
		YPosition.SetPos(0); //5

		AngleRotation.SetRange(0, 360 * AngleSliderCoef, 1);
		AngleRotation.SetPos(0); //41

		Regime.AddString(_T("1. Хаусдорфово расстояние"));
		Regime.AddString(_T("2. Число блоков"));

		NetType.AddString(_T("1. Квадраты"));
		NetType.AddString(_T("2. Шестиугольники"));
		NetType.AddString(_T("3. Описанные окружности (6)"));
		NetType.AddString(_T("4. Описанные окружности (4)"));
		NetType.SetCurSel(3); // задаем значение по умолчанию, чтобы сразу был выбран первый вариант (квадраты)

		CheckRemoveExcess.SetCheck(1);

		EnableDrawNet.SetCheck(0);
		EnableDrawOval.SetCheck(1);
		EnableDrawInts.SetCheck(1);
		//EnableDrawDots.SetCheck(0);
		//EnableDrawHausDot.SetCheck(1);

		FigureType.AddString(_T("1. Овал Кассини"));
		FigureType.AddString(_T("2. Квадрат"));
		FigureType.AddString(_T("3. Треугольник"));
		FigureType.AddString(_T("4. Восьмерка"));
		FigureType.SetCurSel(3);

		CheckRotateNet.SetCheck(0);
		NetXRefSlider.SetRange(-RadiusOfBlock * SliderLimit, RadiusOfBlock * SliderLimit, 1);
		NetYRefSlider.SetRange(-RadiusOfBlock * SliderLimit, RadiusOfBlock * SliderLimit, 1);
		NetAngleSlider.SetRange(0, 360, 1);
		NetXRefSlider.EnableWindow(0);
		NetYRefSlider.EnableWindow(0);
		NetAngleSlider.EnableWindow(0);

		ScaleDrawingSlider.SetRange(scale_drawing - 10, scale_drawing + 30, 1);
		ScaleDrawingSlider.SetPos(scale_drawing);
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CStripsDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CStripsDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this); // device context for painting

		// рисуем сетку
		if (EnableDrawNet.GetCheck() == 1) {
			dc.SelectObject(blackpen);
			if (Net[0].size() > 0)
			{
				StructureForDrawPaths = make_structure_for_draw(StructureForDrawPaths, Net);
				for (int i = 0; i < Net[0].size(); i++) {
					for (int j = 0; j < Net[0][i].size() - 1; j++) {
						dc.MoveTo(StructureForDrawPaths[i][j]);
						dc.LineTo(StructureForDrawPaths[i][j + 1]);
					}
				}
			}
		}

		// рисуем пересечение
		if (EnableDrawInts.GetCheck() == 1) {
			dc.SelectObject(bluepen);
			if (CoveredNet[0].size() > 0)
			{
				StructureForDrawPaths = make_structure_for_draw(StructureForDrawPaths, CoveredNet);
				for (int i = 0; i < CoveredNet[0].size(); i++) {
					for (int j = 0; j < CoveredNet[0][i].size() - 1; j++) {
						dc.MoveTo(StructureForDrawPaths[i][j]);
						dc.LineTo(StructureForDrawPaths[i][j + 1]);
					}
				}
			}
		}

		// рисуем фигуру (овал)
		if (EnableDrawOval.GetCheck() == 1) {
			dc.SelectObject(redpen);		
			if (RotatedAndMovedFigure[0].size() > 0)
			{
				StructureForDrawPaths = make_structure_for_draw(StructureForDrawPaths, RotatedAndMovedFigure);
				for (int i = 0; i < RotatedAndMovedFigure[0].size(); i++) {
					for (int j = 0; j < RotatedAndMovedFigure[0][i].size() - 1; j++) {
						dc.MoveTo(StructureForDrawPaths[i][j]);
						dc.LineTo(StructureForDrawPaths[i][j + 1]);
					}
				}
			}
		}
		//if (EnableDrawOval.GetCheck() == 1) {
		//	dc.SelectObject(redpen);
		//	if (RotatedAndMovedFigure[0].size() > 0)
		//	{
		//		StructureForDrawPath = make_structure_for_draw(StructureForDrawPath, RotatedAndMovedFigure);
		//		dc.Polyline(StructureForDrawPath, RotatedAndMovedFigure[0].size());

		//		dc.MoveTo(StructureForDrawPath[RotatedAndMovedFigure[0].size() - 1]); // замыкаем
		//		dc.LineTo(StructureForDrawPath[0]);
		//	}
		//}

		// рисуем ближайшую точку
		if (EnableDrawHausDot.GetCheck() == 1) {
			dc.SelectObject(redpen);
			if (Net[0].size() > 0) {
				dc.Ellipse(window_center_x + (scale_drawing * XHaus / scale_helper) - 3,
					window_center_y - (scale_drawing * YHaus / scale_helper) - 3,
					window_center_x + (scale_drawing * XHaus / scale_helper) + 3,
					window_center_y - (scale_drawing * YHaus / scale_helper) + 3);

				// и само расстояние Хаусдорфа
				dc.MoveTo(window_center_x + (scale_drawing * XHaus / scale_helper),
					window_center_y - (scale_drawing * YHaus / scale_helper));
				dc.LineTo(window_center_x + (scale_drawing * OvalHaus.X / scale_helper),
					window_center_y - (scale_drawing * OvalHaus.Y / scale_helper));
			}
		}

		// рисуем точки, для которых считаем расстояние хаусдорфа
		if (EnableDrawDots.GetCheck() == 1)
		{
			dc.SelectObject(greenpen);
			for (int i = 0; i < AnalysingHausDistPoints[0].size(); i++)
			{
				dc.Ellipse(window_center_x + (scale_drawing * AnalysingHausDistPoints[0][i].X / scale_helper) - 3,
					window_center_y - (scale_drawing * AnalysingHausDistPoints[0][i].Y / scale_helper) - 3,
					window_center_x + (scale_drawing * AnalysingHausDistPoints[0][i].X / scale_helper) + 3,
					window_center_y - (scale_drawing * AnalysingHausDistPoints[0][i].Y / scale_helper) + 3);					
			}
		}
	}
}

void draw_object(vector<Path> object)
{

}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CStripsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CStripsDlg::add_figure()
{
	Figure[0].clear();

	switch (FigureType.GetCurSel())
	{
		case 0: create_oval_cassini(); break;
		case 1: create_square(); break;
		case 2: create_triangle(); break;
		case 3: create_eight(); break;
	}
	if (Figure[0].size() > 0) rotate_and_move_figure();
}

void CStripsDlg::create_oval_cassini()
{
	double a = 5.5, c = 5;; // параметры овала Кассини
	double xC = sqrt(a * a + c * c), yA = (a * a) / (2 * c); // граничные точки
	double x_current = 0, y_current = 0;

	vector<Path> OnePath(1);

	Figure[0].clear();

	FigureXMax = xC;
	FigureYMax = yA;

	for (x_current = -xC + 0.001; x_current <= xC; x_current += 0.1)
	{
		y_current = sqrt(sqrt(a * a * a * a + 4 * c * c * x_current * x_current) - x_current * x_current - c * c);
		if (y_current >= 0) OnePath[0] << IntPoint(x_current * scale_helper, y_current * scale_helper);
	}

	// добавим самую крайнюю точку, чтобы овал не срезался
	x_current = xC - 0.001;
	y_current = sqrt(sqrt(a * a * a * a + 4 * c * c * x_current * x_current) - x_current * x_current - c * c);

	OnePath[0] << IntPoint(x_current * scale_helper, y_current * scale_helper);

	// рисуем нижнюю часть овала Кассини
	for (int i = OnePath[0].size() - 1; i >= 0; i--)
	{
		OnePath[0] << IntPoint(OnePath[0][i].X, -1 * OnePath[0][i].Y);
	}

	Figure[0].push_back(OnePath[0]);
}

void CStripsDlg::create_square()
{
	// (-3, 3) (3, 3) (3, -3) (-3, -3)
	double SquareLength = 6.0; // длина стороны квадрата
	int NumOfDotsInEdge = 100; // число точек на ребре квадрата

	vector<Path> OnePath(1);

	Figure[0].clear();

	FigureXMax = SquareLength/2;
	FigureYMax = SquareLength/2;

	OnePath[0] << IntPoint(-SquareLength/2 * scale_helper, SquareLength/2 * scale_helper);
	for (double i = -SquareLength/2 + SquareLength / NumOfDotsInEdge; i < SquareLength/2; i += SquareLength / NumOfDotsInEdge)
	{
		OnePath[0] << IntPoint(i * scale_helper, SquareLength/2 * scale_helper);
	}

	OnePath[0] << IntPoint(SquareLength/2 * scale_helper, SquareLength/2 * scale_helper);
	for (double i = SquareLength/2 - SquareLength / NumOfDotsInEdge; i > -SquareLength/2; i -= SquareLength / NumOfDotsInEdge)
	{
		OnePath[0] << IntPoint(SquareLength/2 * scale_helper, i * scale_helper);
	}

	OnePath[0] << IntPoint(SquareLength/2 * scale_helper, -SquareLength/2 * scale_helper);
	for (double i = SquareLength/2 - SquareLength / NumOfDotsInEdge; i > -SquareLength/2; i -= SquareLength / NumOfDotsInEdge)
	{
		OnePath[0] << IntPoint(i * scale_helper, -SquareLength/2 * scale_helper);
	}

	OnePath[0] << IntPoint(-SquareLength/2 * scale_helper, -SquareLength/2 * scale_helper);
	for (double i = -SquareLength/2 + SquareLength / NumOfDotsInEdge; i < SquareLength/2; i += SquareLength / NumOfDotsInEdge)
	{
		OnePath[0] << IntPoint(-SquareLength/2 * scale_helper, i * scale_helper);
	}

	Figure[0].push_back(OnePath[0]);
}

void CStripsDlg::create_triangle()
{
	doublePoint EdgePoint1, EdgePoint2, EdgePoint3; // вершины треугольника
	doublePoint currentPoint;
	int NumOfDotsInEdge = 100; // число точек на ребре треугольника

	vector<Path> OnePath(1);

	Figure[0].clear();

	EdgePoint1.x = 0;
	EdgePoint1.y = 3;

	EdgePoint2.x = 1.5*sqrt(3);
	EdgePoint2.y = -0.5;

	EdgePoint3.x = -1.5*sqrt(3);
	EdgePoint3.y = -0.5;

	OnePath[0] << IntPoint(EdgePoint1.x * scale_helper, EdgePoint1.y * scale_helper);
	for (double x = EdgePoint1.x + (EdgePoint2.x - EdgePoint1.x) / NumOfDotsInEdge; x < EdgePoint2.x; x += (EdgePoint2.x - EdgePoint1.x) / NumOfDotsInEdge)
	{
		currentPoint.x = x;
		currentPoint.y = (currentPoint.x * (EdgePoint2.y - EdgePoint1.y) - EdgePoint1.x * (EdgePoint2.y - EdgePoint1.y)) / (EdgePoint2.x - EdgePoint1.x) + EdgePoint1.y;
		OnePath[0] << IntPoint(currentPoint.x * scale_helper, currentPoint.y * scale_helper);
	}

	OnePath[0] << IntPoint(EdgePoint2.x * scale_helper, EdgePoint2.y * scale_helper);
	for (double x = EdgePoint2.x - (EdgePoint3.x - EdgePoint2.x) / NumOfDotsInEdge; x > EdgePoint3.x; x += (EdgePoint3.x - EdgePoint2.x) / NumOfDotsInEdge)
	{
		currentPoint.x = x;
		currentPoint.y = (currentPoint.x * (EdgePoint3.y - EdgePoint2.y) - EdgePoint2.x * (EdgePoint3.y - EdgePoint2.y)) / (EdgePoint3.x - EdgePoint2.x) + EdgePoint2.y;
		OnePath[0] << IntPoint(currentPoint.x * scale_helper, currentPoint.y * scale_helper);
	}

	OnePath[0] << IntPoint(EdgePoint3.x * scale_helper, EdgePoint3.y * scale_helper);
	for (double x = EdgePoint3.x + (EdgePoint1.x - EdgePoint3.x) / NumOfDotsInEdge; x < EdgePoint1.x; x += (EdgePoint1.x - EdgePoint3.x) / NumOfDotsInEdge)
	{
		currentPoint.x = x;
		currentPoint.y = (currentPoint.x * (EdgePoint1.y - EdgePoint3.y) - EdgePoint3.x * (EdgePoint1.y - EdgePoint3.y)) / (EdgePoint1.x - EdgePoint3.x) + EdgePoint3.y;
		OnePath[0] << IntPoint(currentPoint.x * scale_helper, currentPoint.y * scale_helper);
	}

	Figure[0].push_back(OnePath[0]);
}

void CStripsDlg::create_eight()
{
	double R = 3.0;
	double r = R * (sqrt(2) - 1);
	double a = 1.5;

	double x_current = 0, y_current = 0;

	vector<Path> OnePath(1);

	Figure[0].clear();

	for (double x = -2 * R; x < R * cos(PI / 4) - R; x += 0.1)
	{
		x_current = x;
		y_current = sqrt(pow(R, 2) - pow((x_current + R), 2));
		OnePath[0] << IntPoint(x_current * scale_helper, y_current * scale_helper);
	}

	for (double x = R * cos(PI / 4) - R; x < R - R * cos(PI / 4); x += 0.1)
	{
		x_current = x;
		y_current = -1 * sqrt(pow(r, 2) - pow(x_current, 2)) + R;
		OnePath[0] << IntPoint(x_current * scale_helper, y_current * scale_helper);
	}

	for (double x = R - R * cos(PI / 4); x <= 2 * R; x += 0.1)
	{
		x_current = x;
		y_current = sqrt(pow(R, 2) - pow((x_current - R), 2));
		OnePath[0] << IntPoint(x_current * scale_helper, y_current * scale_helper);
	}

	for (int i = OnePath[0].size() - 1; i >= 0; i--)
	{
		OnePath[0] << IntPoint(OnePath[0][i].X, -1 * OnePath[0][i].Y);
	}

	Figure[0].push_back(OnePath[0]);

	OnePath[0].clear();

	for (double x = -R + (R / a); x >= -R - (R/a); x -= 0.1)
	{
		x_current = x;
		y_current = sqrt(pow((R / a), 2) - pow((x_current + R), 2));
		if (y_current >= 0) OnePath[0] << IntPoint(x_current * scale_helper, y_current * scale_helper);
	}

	for (int i = OnePath[0].size() - 1; i >= 0; i--)
	{
		OnePath[0] << IntPoint(OnePath[0][i].X, -1 * OnePath[0][i].Y);
	}

	Figure[0].push_back(OnePath[0]);

	OnePath[0].clear();

	for (double x = R + (R / a); x >= R - (R / a); x -= 0.1)
	{
		x_current = x;
		y_current = sqrt(pow((R / a), 2) - pow((x_current - R), 2));
		if (y_current >= 0) OnePath[0] << IntPoint(x_current * scale_helper, y_current * scale_helper);
	}

	for (int i = OnePath[0].size() - 1; i >= 0; i--)
	{
		OnePath[0] << IntPoint(OnePath[0][i].X, -1 * OnePath[0][i].Y);
	}

	Figure[0].push_back(OnePath[0]);
}

void CStripsDlg::rotate_and_move_figure()
{
	// смещаем и поворачиваем (оператор поворота: x' = x*cos(alpha) - y*sin(alpha); y'= x*sin(alpha) + y*cos(alpha))
	double x_current = 0, y_current = 0;

	std::vector<Path> OnePath(1);

	RotatedAndMovedFigure[0].clear();

	for (int i = 0; i < Figure[0].size(); i++)
	{
		for (int j = 0; j < Figure[0][i].size(); j++)
		{
			x_current = (Figure[0][i][j].X * cos(AngleRotation.GetPos()*(PI / 180)) - Figure[0][i][j].Y * sin(AngleRotation.GetPos()*(PI / 180))) +
				(XPosition.GetPos() * scale_helper / (SliderCoreff * SliderLimit));

			y_current = (Figure[0][i][j].X * sin(AngleRotation.GetPos()*(PI / 180)) + Figure[0][i][j].Y * cos(AngleRotation.GetPos()*(PI / 180))) +
				(YPosition.GetPos() * scale_helper / (SliderCoreff * SliderLimit));

			OnePath[0] << IntPoint(x_current, y_current);
		}
		RotatedAndMovedFigure[0].push_back(OnePath[0]);
		OnePath[0].clear();
	}	

	set_editcontrol(double(XPosition.GetPos()) / (SliderCoreff * SliderLimit), CurrXPos);
	set_editcontrol(double(YPosition.GetPos()) / (SliderCoreff * SliderLimit), CurrYPos);
	set_editcontrol(double(AngleRotation.GetPos()), CurrAnglePos);
}

// создаем сетку
void CStripsDlg::add_net()
{
	double w = 0; // шаг между центрами соседних блоков по гризонтали
	double h = 0; // шаг между центрами соседних блоков по вертикали
	double l = 1; // длина стороны правильного многоугольника блока

	double init_x = 0, init_y = 0; // начало формирования сетки
	double end_x = 0, end_y = 0; // конец формирования сетки
	double step_x = 0, step_y = 0; // шаг между центрами блоков

	double LimitBorder = 0; // для ОДНОКРАТНОГО создания заранее покрывающей сетки для всех позиций и поворотов

	double x_current = 0, y_current = 0;

	int LineCounter = 0;

	Net[0].clear();
	Areas.clear();

	Intersections[0].clear();
	CentersOfBlocks.clear();

	switch (NetType.GetCurSel())
	{
		case 0: {
			w = l;
			h = l;
		} break; // для квадратов (по умолчанию 1 х 1)
		case 1: {
			w = l * 3;
			h = l * sqrt(3) / 2;
		} break; // шестиугольник с длиной стороны l (по умолчанию l = 1)
		case 2: {
			w = RadiusOfBlock * 3;
			h = RadiusOfBlock * sqrt(3) / 2;
		} break; // описанные круги вокруг шестиугольников
		case 3: {
			w = RadiusOfBlock * sqrt(2);
			h = RadiusOfBlock * sqrt(2);
		} break; // описанные круги вокруг квадратов
	}

	LimitBorder = max(abs(FigureXMax), abs(FigureYMax));
	end_x = 0;
	end_y = 0;
	while (end_x <= LimitBorder + w + 3)
	{
		end_x += w;
	}
	while (end_y <= LimitBorder + h + 3)
	{
		end_y += h;
	}

	init_x = -1 * end_x;
	init_y = -1 * end_y;

	step_x = w;
	step_y = h;

	x_current = init_x;
	y_current = init_y;

	// начинаем рисовать сетку. Начало координат в середине окна
	switch (NetType.GetCurSel())
	{
		case 0: {
			while (y_current < end_y)
			{
				add_block(x_current, y_current, step_x, step_y);
				x_current += step_x;
				if (x_current > end_x - 1)
				{
					x_current = init_x;
					y_current += step_y;
				}
			}
		} break; // для квадратов
		case 1: {
			LineCounter = 0;
			while (y_current < end_y)
			{
				add_block(x_current, y_current, step_x, step_y);
				x_current += step_x;
				if (x_current > end_x - 1)
				{
					LineCounter++;
					if (LineCounter % 2 != 0)
					{
						x_current = init_x + 1.5 * RadiusOfBlock;
					}
					else
					{
						x_current = init_x;
					}
					y_current += step_y;
				}
			}
		} break;
		case 2: {
			LineCounter = 0;
			while (y_current < end_y)
			{
				add_block(x_current, y_current, step_x, step_y);
				x_current += step_x;
				if (x_current > end_x)
				{
					LineCounter++;
					if (LineCounter % 2 != 0)
					{
						x_current = init_x + 1.5 * RadiusOfBlock;
					}
					else
					{
						x_current = init_x;
					}
					y_current += step_y;
				}
			}
		} break; // для шестиугольников
		case 3: {
			while (y_current < end_y)
			{
				add_block(x_current, y_current, step_x, step_y);
				x_current += step_x;
				if (x_current > end_x)
				{
					x_current = init_x;
					y_current += step_y;
				}
			}
		} break;
	};
}

// создание отдельного блока для сетки
void CStripsDlg::add_block(double init_value_x, double init_value_y, double step_value_x, double step_value_y)
{
	IntPoint center;

	Block[0].clear();

	switch (NetType.GetCurSel())
	{
	case 0:
		{
			// по часовой стрелке с левого верхнего угла
			int DotsOnBlockEdge = 6;

			// верхняя линия
			Block[0] << IntPoint((init_value_x - 0.5) * scale_helper, (init_value_y - 0.5) * scale_helper);
			for (int i = 1; i < DotsOnBlockEdge + 1; i++) {
				Block[0] << IntPoint((init_value_x + (1 / (double(DotsOnBlockEdge) + 1)) * i)* scale_helper, init_value_y * scale_helper);
			}

			// правая вертикальная линия
			Block[0] << IntPoint((init_value_x + 0.5) * scale_helper, (init_value_y - 0.5) * scale_helper);
			for (int i = 1; i < DotsOnBlockEdge + 1; i++) {
				Block[0] << IntPoint((init_value_x + step_value_y) * scale_helper, (init_value_y - (1 / (double(DotsOnBlockEdge) + 1)) * i) * scale_helper);
			}

			// нижняя линия
			Block[0] << IntPoint((init_value_x + 0.5) * scale_helper, (init_value_y + 0.5) * scale_helper);
			for (int i = 1; i < DotsOnBlockEdge + 1; i++) {
				Block[0] << IntPoint((init_value_x + step_value_x - (1 / (double(DotsOnBlockEdge) + 1)) * i) * scale_helper, (init_value_y - step_value_y) * scale_helper);
			}

			// левая линия
			Block[0] << IntPoint((init_value_x - 0.5) * scale_helper, (init_value_y + 0.5) * scale_helper);
			for (int i = 1; i < DotsOnBlockEdge + 1; i++) {
				Block[0] << IntPoint(init_value_x  * scale_helper, (init_value_y - step_value_y + (1 / (double(DotsOnBlockEdge) + 1)) * i) * scale_helper);
			}
		} break; // для квадратов
		case 1:
		{
			//for (int i = 0; i < 360; i += 60) {
			//	Block[0] << IntPoint((init_value_x + l * cos(i*PI / 180))*scale_helper, (init_value_y + l * sin(i*PI / 180))*scale_helper);
			//}
		} break; // для шестиугольников
		case 2:
		{
			for (int i = 0; i < 360; i += 1) {
				Block[0] << IntPoint((init_value_x + RadiusOfBlock * cos(i*PI / 180))*scale_helper, (init_value_y + RadiusOfBlock * sin(i*PI / 180))*scale_helper);
			}
		} break; // для кругов
		case 3:
		{
			for (int i = 0; i < 360; i += 1) {
				Block[0] << IntPoint((init_value_x + RadiusOfBlock * cos(i * PI / 180)) * scale_helper, (init_value_y + RadiusOfBlock * sin(i * PI / 180)) * scale_helper);
			}
		} break;
	};

	init_value_x_rotated = init_value_x;
	init_value_y_rotated = init_value_y;

	if(CheckRotateNet.GetCheck() == 1) rotate_and_move_net(init_value_x, init_value_y,
						double(-1 * NetXRefSlider.GetPos())/SliderLimit, double(-1 * NetYRefSlider.GetPos()) / SliderLimit, NetAngleSlider.GetPos());

	Net[0].push_back(Block[0]);

	center.X = init_value_x_rotated * scale_helper;
	center.Y = init_value_y_rotated * scale_helper;
	CentersOfBlocks.push_back(center);
}

void CStripsDlg::rotate_and_move_net(double init_value_x, double init_value_y, double ref_x, double ref_y, double Angle)
{
	vector<Path> BlockRotated(1);

	for (int i = 0; i < Block[0].size(); i++) {
		BlockRotated[0] << IntPoint((Block[0][i].X * cos(Angle * (PI / 180)) - Block[0][i].Y * sin(Angle * (PI / 180))) +
			((-SliderLimit * ref_x) * scale_helper / (SliderCoreff * SliderLimit)),
			(Block[0][i].X * sin(Angle * (PI / 180)) + Block[0][i].Y * cos(Angle * (PI / 180))) +
			((-SliderLimit * ref_y) * scale_helper / (SliderCoreff * SliderLimit)));
	}

	init_value_x_rotated = init_value_x * cos(Angle * (PI / 180)) - init_value_y * sin(Angle * (PI / 180)) +
		((-SliderLimit * ref_x) * scale_helper / (SliderCoreff * SliderLimit));
	init_value_y_rotated = init_value_x * sin(Angle * (PI / 180)) + init_value_y * cos(Angle * (PI / 180)) +
		((-SliderLimit * ref_y) * scale_helper / (SliderCoreff * SliderLimit));

	Block[0] = BlockRotated[0];
}

// пересекаем овал Кассини и сетку
void CStripsDlg::add_intersec_figure_and_net()
{
	vector<Path> CurrentIntersection(1);

	Intersections[0].clear();
	CoveredNet[0].clear();
	CoveredsAreas.clear();
	CentersOfCoveredBlocks.clear();

	if(Figure[0].size() == 0) add_figure();
	if(Net[0].size() == 0) add_net();

	for (int i = 0; i < Net[0].size(); i++)
	{
		CurrentIntersection = do_clip_action(Net[0], RotatedAndMovedFigure[0], ctIntersection, i);
		if (CurrentIntersection.size() > 0)
		{
			Intersections[0].push_back(CurrentIntersection[0]);
			Areas.push_back(Area(CurrentIntersection[0]) / pow(scale_helper, 2));
			CoveredsAreas.push_back(Area(CurrentIntersection[0]) / pow(scale_helper, 2));
			CoveredNet[0].push_back(Net[0][i]);
			CentersOfCoveredBlocks.push_back(CentersOfBlocks[i]);
		}
		else
		{
			Areas.push_back(0);
		}
	}

	BlockArea = abs(Area(CoveredNet[0][0])) / (scale_helper * scale_helper);

	if (CheckRemoveExcess.GetCheck() == 1) remove_excess_blocks();

	set_editcontrol(CoveredNet[0].size(), NumOfCoveredBlocks);
}

void CStripsDlg::remove_excess_blocks()
{
	set_analysing_points();
	crossed_dots_nearest_blocks();
	analysis_and_remove_excess_blocks();
}

void CStripsDlg::set_analysing_points()
{
	vector<Path> EmptyPath(1);
	vector<Path> CurrentPoint(1);
	bool WeAreInside;

	CrossedDots[0].clear();
	CheckedDots_new[0].clear();

	for (int i = 0; i < CoveredNet[0].size(); i++)
	{
		CheckedDots_new[0].push_back(EmptyPath[0]);
		CurrentPoint[0].clear();
		if (int(CoveredsAreas[i] * 100 + 0.5) != int(BlockArea * 100))
		{
			for (int j = 0; j < CoveredNet[0][i].size(); j++)
			{
				if (abs(PointInPolygon(CoveredNet[0][i][j], RotatedAndMovedFigure[0][0])))
				{
					if (j == 0)
					{
						WeAreInside = true;
					}
					if (!WeAreInside)
					{
						if (j == 0) {
							CurrentPoint[0] << IntPoint((CoveredNet[0][i][j].X + CoveredNet[0][i][CoveredNet[0][i].size() - 1].X) / 2,
								(CoveredNet[0][i][j].Y + CoveredNet[0][i][CoveredNet[0][i].size() - 1].Y) / 2);
						}
						else
						{
							CurrentPoint[0] << IntPoint((CoveredNet[0][i][j].X + CoveredNet[0][i][j - 1].X) / 2,
								(CoveredNet[0][i][j].Y + CoveredNet[0][i][j - 1].Y) / 2);
						}
						WeAreInside = true;
					}
				}
				else
				{
					if (j == 0)
					{
						WeAreInside = false;
					}
					if (WeAreInside)
					{
						if (j == 0) {
							CurrentPoint[0] << IntPoint((CoveredNet[0][i][j].X + CoveredNet[0][i][CoveredNet[0][i].size() - 1].X) / 2,
								(CoveredNet[0][i][j].Y + CoveredNet[0][i][CoveredNet[0][i].size() - 1].Y) / 2);
						}
						else
						{
							CurrentPoint[0] << IntPoint((CoveredNet[0][i][j].X + CoveredNet[0][i][j - 1].X) / 2,
								(CoveredNet[0][i][j].Y + CoveredNet[0][i][j - 1].Y) / 2);
						}
						WeAreInside = false;
					}
					CheckedDots_new[0][i].push_back(CoveredNet[0][i][j]); // добавляем точку в реестр
				}
			}
			if (((abs(PointInPolygon(CoveredNet[0][i][0], RotatedAndMovedFigure[0][0]))) && !(WeAreInside)) || // на случай, если переход на
				!(abs(PointInPolygon(CoveredNet[0][i][0], RotatedAndMovedFigure[0][0]))) && (WeAreInside))	 // первой и последней точке блока
			{
				CurrentPoint[0] << IntPoint((CoveredNet[0][i][0].X + CoveredNet[0][i][CoveredNet[0][i].size() - 1].X) / 2,
					(CoveredNet[0][i][0].Y + CoveredNet[0][i][CoveredNet[0][i].size() - 1].Y) / 2);
			}
			//if ((CurrDot[0].size() == 1) && !(abs(PointInPolygon(CentersOfCoveredBlocks[i], RotatedAndMovedFigure[0])))) // костыль, но пока только так
			//{
			//	NumToDelete.push_back(i);
			//}
		}
		CrossedDots[0].push_back(CurrentPoint[0]);
	}
}

void CStripsDlg::crossed_dots_nearest_blocks()
{
	InfoOFBlockCrossedOval CurrentInfo;
	double CurrentDistance;
	double NativeDistance = RadiusOfBlock * scale_helper;

	CrossedBlocksInfo.clear();

	for (int i = 0; i < CoveredNet[0].size(); i++)
	{
		if (!(abs(PointInPolygon(CentersOfCoveredBlocks[i], RotatedAndMovedFigure[0][0]))))
		{
			for (int j = 0; j < CrossedDots[0][i].size(); j++)
			{
				for (int k = 0; k < CentersOfCoveredBlocks.size(); k++)
				{
					if (i == k)
					{
						continue;
					}
					CurrentDistance = sqrt(pow(CrossedDots[0][i][j].X - CentersOfCoveredBlocks[k].X, 2) + pow(CrossedDots[0][i][j].Y - CentersOfCoveredBlocks[k].Y, 2));
					if (CurrentDistance < NativeDistance)
					{
						CurrentInfo.BlockID = i;
						CurrentInfo.DotID = j;
						CurrentInfo.NearestID = k;
						CrossedBlocksInfo.push_back(CurrentInfo);
					}
				}
			}
		}
	}
}

void CStripsDlg::analysis_and_remove_excess_blocks()
{
	int DeleteCounter = 0;
	bool SuchAlreadyHere = false;

	vector<int> DeleteBlock; // ID блока из массива CoveredNet, который нужно удалить из покрытия
	if (CrossedBlocksInfo.size() > 0) {
		for (int i = 0; i < CrossedBlocksInfo.size() - 1; i++)
		{
			for (int j = i + 1; j < i + CrossedDots[0][CrossedBlocksInfo[i].BlockID].size(); j++)
			{
				if ((CrossedBlocksInfo[i].BlockID == CrossedBlocksInfo[j].BlockID)
					&& (CrossedBlocksInfo[i].NearestID == CrossedBlocksInfo[j].NearestID))
				{
					for (int k = 0; k < DeleteBlock.size(); k++)
					{
						if (DeleteBlock[k] == CrossedBlocksInfo[i].BlockID)
						{
							SuchAlreadyHere = true;
							break;
						}
					}
					if (SuchAlreadyHere)
					{
						SuchAlreadyHere = false;
					}
					else
					{
						DeleteBlock.push_back(CrossedBlocksInfo[i].BlockID);
					}
				}
			}
		}
	}

	for (int i = 0; i < DeleteBlock.size(); i++)
	{
		CoveredNet[0].erase(next(CoveredNet[0].begin(), DeleteBlock[i] - DeleteCounter));
		CoveredsAreas.erase(next(CoveredsAreas.begin(), DeleteBlock[i] - DeleteCounter));
		CheckedDots_new[0].erase(next(CheckedDots_new[0].begin(), DeleteBlock[i] - DeleteCounter));

		DeleteCounter++;
	}
}

// считаем Хаусдорфово расстояние
void CStripsDlg::find_haus_dist()
{
	double CurrentDistance;
	double HausDistance = 0;
	double CurrentNormalLength;
	double MinNormalLength = (2 * RadiusOfBlock + 1) * scale_helper; // 2, потому что полной диаметр окружности - максимальное расстояние

	prepare_points_to_find_Haus();

	for (int i = 0; i < AnalysingHausDistPoints[0].size(); i++)
	{
		MinNormalLength = (2 * RadiusOfBlock + 1) * scale_helper;
		for (int j = 0; j < RotatedAndMovedFigure[0].size(); j++)
		{
			CurrentNormalLength = sqrt(pow(AnalysingHausDistPoints[0][i].X - RotatedAndMovedFigure[0][0][j].X, 2)
				+ pow(AnalysingHausDistPoints[0][i].Y - RotatedAndMovedFigure[0][0][j].Y, 2));
			if (CurrentNormalLength < MinNormalLength)
			{
				MinNormalLength = CurrentNormalLength;

				OvalHausPrev.X = RotatedAndMovedFigure[0][0][j].X;
				OvalHausPrev.Y = RotatedAndMovedFigure[0][0][j].Y;
			}
		}

		CurrentDistance = MinNormalLength; // строка и переменная лишние, но для лучшего понимания

		if (CurrentDistance > HausDist)
		{
			HausDist = CurrentDistance;

			XHaus = AnalysingHausDistPoints[0][i].X;
			YHaus = AnalysingHausDistPoints[0][i].Y;

			OvalHaus.X = OvalHausPrev.X;
			OvalHaus.Y = OvalHausPrev.Y;
		}
	}

	HausDist /= scale_helper;

	set_editcontrol(HausDist, HausdorffDistance);
}

void CStripsDlg::prepare_points_to_find_Haus()
{
	vector<Path> CurrRes;
	vector<int> RetryUnion;

	AnalysingHausDistPoints[0].clear();
	AnalysingHausDistPoints[0] = CoveredNet[0][0];

	//int old = 0;

	//for (int i = 0; i < CheckedDots_new[0].size(); i++)
	//{
	//	old += CheckedDots_new[0][i].size();
	//}

	for (int i = 1; i < CoveredNet[0].size(); i++)
	{
		CurrRes = do_clip_action(CoveredNet[0], AnalysingHausDistPoints, ctUnion, i);
		if (CurrRes.size() == 1)
		{
			AnalysingHausDistPoints[0] = CurrRes[0];
		}
		else
		{
			if ((CurrRes.size() > 1) && (CurrRes[1].size() < 360))
			{
				AnalysingHausDistPoints[0] = CurrRes[0];
			}
			else
			{
				RetryUnion.push_back(i);
			}
		}
		//AnalysingHausDistPoints[0] = do_clip_action(CoveredNet[0], AnalysingHausDistPoints, ctUnion)[0];
		//draw_everything();
	}

	for (int i = RetryUnion.size() - 1; i >= 0; i--)
	{
		CurrRes = do_clip_action(CoveredNet[0], AnalysingHausDistPoints, ctUnion, RetryUnion[i]);
		if (CurrRes.size() == 1)
		{
			AnalysingHausDistPoints[0] = CurrRes[0];
		}
		else
		{
			if ((CurrRes.size() > 1) && (CurrRes[1].size() < 360))
			{
				AnalysingHausDistPoints[0] = CurrRes[0];
			}
			else
			{
				RetryUnion.push_back(i);
			}
		}
		//draw_everything();
	}
	//int New = AnalysingHausDistPoints[0].size();
}

// ищем минмальное Хаусдорфово расстояние
void CStripsDlg::do_check_all_positions()
{
	double MinHausDist = (RadiusOfBlock * 2 + 1) * scale_helper;
	int NumOfBlocks = 0;

	// зафиксируем, в каком случае достигается минимальное значение Хаусдорфова расстояния
	int MinXPos = 0;
	int MinYPos = 0;
	int MinAnglePos = 0;

	// для однократного формирования сетки
	if(Figure[0].size() <= 0) add_figure();
	if(Net[0].size() <= 0) add_net();

	NumOfBlocks = Net[0].size();

	for (int i = XPosition.GetRangeMin(); i <= XPosition.GetRangeMax(); i++) {
		XPosition.SetPos(i);
		for (int j = YPosition.GetRangeMin(); j <= YPosition.GetRangeMax(); j++) {
			YPosition.SetPos(j);
			for (int k = 0; k <= 90; k += 1) {
				AngleRotation.SetPos(k);

				rotate_and_move_figure();
				add_intersec_figure_and_net();

				switch (Regime.GetCurSel())
				{ // проверяем, что мы сейчас считаем
					case 0:
					{
						find_haus_dist();

						if (HausDist < MinHausDist) {
							MinHausDist = HausDist;

							MinXPos = XPosition.GetPos();
							MinYPos = YPosition.GetPos();
							MinAnglePos = AngleRotation.GetPos();
						}
					} break;
					case 1:
					{
						if (CoveredNet[0].size() < NumOfBlocks) {
							NumOfBlocks = CoveredNet[0].size();

							MinXPos = XPosition.GetPos();
							MinYPos = YPosition.GetPos();
							MinAnglePos = AngleRotation.GetPos();
						}
					} break;
				}
			}
		}
	}

	// установим найденные условия
	XPosition.SetPos(MinXPos);
	YPosition.SetPos(MinYPos);
	AngleRotation.SetPos(MinAnglePos);

	set_editcontrol(double(XPosition.GetPos()) / (SliderCoreff * SliderLimit), CurrXPos);
	set_editcontrol(double(YPosition.GetPos()) / (SliderCoreff * SliderLimit), CurrYPos);
	set_editcontrol(double(AngleRotation.GetPos()), CurrAnglePos);

	rotate_and_move_figure();
	add_intersec_figure_and_net();
	find_haus_dist();

	EnableDrawOval.SetCheck(1);
	EnableDrawInts.SetCheck(1);
	EnableDrawHausDot.SetCheck(1);
	draw_everything();
}

vector<Path> CStripsDlg::do_clip_action(vector<Path> who_clip, vector<Path> who_clipped, ClipType clip_action, int num_of_who_clip, int num_of_who_clipped)
{
	Clipper cp;
	vector<Path> CurrRes(1);

	cp.AddPath(who_clip[num_of_who_clip], ptSubject, true);
	cp.AddPath(who_clipped[num_of_who_clipped], ptClip, true);
	cp.Execute(clip_action, CurrRes, pftNonZero, pftNonZero);

	return CurrRes;
}

// рисуем все, что создали
void CStripsDlg::draw_everything()
{
	InvalidateRect(RedrawArea, 1);
	OnPaint();
}

// настраиваем слайдер и поле значения для него
void CStripsDlg::set_editcontrol(double value, CEdit& window)
{
	CString TextForCtrl = _T("");

	TextForCtrl.Format(_T("%.3f"), value);
	window.SetWindowTextW(TextForCtrl);
}

// реагируем на движение слайдера
void CStripsDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CSliderCtrl *pSlider = reinterpret_cast<CSliderCtrl*>(pScrollBar);

	if (pSlider == &XPosition || pSlider == &YPosition || pSlider == &AngleRotation || pSlider == &ScaleOfOval)
	{		
		rotate_and_move_figure();
		//add_intersec_figure_and_net();
	}

	if (pSlider == &NetXRefSlider || pSlider == &NetYRefSlider || pSlider == &NetAngleSlider)
	{
		add_net();
		add_intersec_figure_and_net();
	}

	if (pSlider == &ScaleDrawingSlider)
	{
		scale_drawing = ScaleDrawingSlider.GetPos();
		set_drawing_param(window_center_x, window_center_y, scale_drawing, scale_helper);
	}

	draw_everything();

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CStripsDlg::OnBnClickedCheck2()
{
	draw_everything();
}

void CStripsDlg::OnBnClickedCheck3()
{
	draw_everything();
}

void CStripsDlg::OnBnClickedCheck4()
{
	draw_everything();
}

void CStripsDlg::OnBnClickedCheck6()
{
	draw_everything();
}

void CStripsDlg::OnBnClickedCheck9()
{
	add_intersec_figure_and_net();
	draw_everything();
}

void CStripsDlg::OnBnClickedCheck7()
{
	draw_everything();
}

void CStripsDlg::OnBnClickedCheck10()
{
	if (CheckRotateNet.GetCheck() == 1)
	{
		NetXRefSlider.EnableWindow(1);
		NetYRefSlider.EnableWindow(1);
		NetAngleSlider.EnableWindow(1);
	}
	else
	{
		NetXRefSlider.EnableWindow(0);
		NetYRefSlider.EnableWindow(0);
		NetAngleSlider.EnableWindow(0);
	}
}
