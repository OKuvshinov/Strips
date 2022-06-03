
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

using namespace ClipperLib;

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
	DDX_Control(pDX, IDC_CHECK1, do_ints);
	DDX_Control(pDX, IDC_CHECK2, EnableDrawNet);
	DDX_Control(pDX, IDC_CHECK3, EnableDrawOval);
	DDX_Control(pDX, IDC_CHECK4, EnableDrawInts);
	DDX_Control(pDX, IDC_CHECK5, EnableDrawDots);
	DDX_Control(pDX, IDC_CHECK6, EnableDrawHausDot);
}

BEGIN_MESSAGE_MAP(CStripsDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CStripsDlg::add_net)
	ON_BN_CLICKED(IDC_BUTTON2, &CStripsDlg::draw_everything)
	ON_BN_CLICKED(IDC_BUTTON3, &CStripsDlg::add_oval)
	ON_BN_CLICKED(IDC_BUTTON4, &CStripsDlg::add_intersec_oval_and_net)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON5, &CStripsDlg::find_haus_dist)
	ON_BN_CLICKED(IDC_BUTTON6, &CStripsDlg::do_check_all_positions)
	ON_BN_CLICKED(IDC_CHECK2, &CStripsDlg::OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK3, &CStripsDlg::OnBnClickedCheck3)
	ON_BN_CLICKED(IDC_CHECK4, &CStripsDlg::OnBnClickedCheck4)
	ON_BN_CLICKED(IDC_CHECK5, &CStripsDlg::OnBnClickedCheck5)
	ON_BN_CLICKED(IDC_CHECK6, &CStripsDlg::OnBnClickedCheck6)
END_MESSAGE_MAP()


// CStripsDlg message handlers

Clipper cp;
std::vector<Path> CurrInts(1); // внутренний (для общей функции пересечения) промежуточный результат
								// для функции пересечения (чтобы каждый раз не объявлять новый)
std::vector<Path> CurrentIntersection(1); // внешний промежуточный результат (для конкретной функции).
											//Можно было бы и без него, но так код читабельнее

CRect RedrawArea;

// масштаб для рисования. Оптимальное значение - 20. Изменение сетки будем имитировать изменением размеров овала Кассини
// для этого будем "играться" с масштабом отрисовки.
double scale_2 = 5 / 1;
double scale_1 = 100;
int scale = scale_1 / scale_2;

// чтобы не потерять точность
int scale_helper = 1000;

int window_center_x = 0;
int window_center_y = 0;

/////////////// параметры блоков сетки /////////////////
double w = 0; // шаг между центрами соседних блоков по гризонтали
double h = 0; // шаг между центрами соседних блоков по вертикали

double l = 1; // длина стороны правильного многоугольника блока


int SliderLimit = 5; // диапазон для слайдеров X и Y

CString TextForCtrl = _T("");

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

		RedrawArea.SetRect(window_center_x - 10 * scale, window_center_y - 10 * scale, window_center_x + 10 * scale, window_center_y + 10 * scale);
		set_drawing_param(window_center_x, window_center_y, scale, scale_helper);

		XPosition.SetRange(-SliderLimit, SliderLimit, 1);
		XPosition.SetPos(0);

		YPosition.SetRange(-SliderLimit, SliderLimit, 1);
		YPosition.SetPos(0);

		AngleRotation.SetRange(0, 360, 1);
		AngleRotation.SetPos(0);

		Regime.AddString(_T("1. Хаусдорфово расстояние"));
		Regime.AddString(_T("2. Число блоков"));

		NetType.AddString(_T("1. Квадраты"));
		NetType.AddString(_T("2. Шестиугольники"));
		NetType.AddString(_T("3. Описанные окружности (6)"));
		NetType.AddString(_T("4. Описанные окружности (4)"));
		NetType.SetCurSel(3); // задаем значение по умолчанию, чтобы сразу был выбран первый вариант (квадраты)

		do_ints.SetCheck(1);

		EnableDrawNet.SetCheck(0);
		EnableDrawOval.SetCheck(1);
		EnableDrawInts.SetCheck(1);
		EnableDrawDots.SetCheck(0);
		EnableDrawHausDot.SetCheck(1);
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

std::vector<Paths> Net(1); // сетка покрытия
std::vector<Paths> CoveredNet(1); // только те элементы сетки, которые покрывают овал

std::vector<Paths> NetRotated(1);

std::vector<Path> OvalKassini(1); // изначальный в начале координат. На его основании поворачиваем и двигаем
std::vector<Path> RotatedAndMovedOvalKassini(1);
std::vector<Paths> Intersections(1);
std::vector<Paths> DotsOfCurrBlock(1);

std::vector<Paths> AllDots(1);

std::vector<Path> Block(1);

int DotsOnEdge = 0; // число точек на грани блкоа сетки

// чтобы рисовать объекты типа Clipperlib::vector<Path> их надо превартить в структуры типа POINT
POINT ** StructureForDrawPaths;
POINT * StructureForDrawPath;

CPen blackpen(PS_SOLID, 1, RGB(0, 0, 0));
CPen redpen(PS_SOLID, 1, RGB(255, 0, 0));
CPen bluepen(PS_SOLID, 1, RGB(0, 0, 255));

int XHaus = 0; // какая точка соответствует Хаусдорфовому расстоянию
int YHaus = 0;

int XOvalHausPrev = 0; // точка на овале Кассини, соответствующая расстоянию Хаусдорфа
int YOvalHausPrev = 0;

int XOvalHaus = 0; // точка на овале Кассини, соответствующая расстоянию Хаусдорфа
int YOvalHaus = 0; // точка на овале Кассини, соответствующая расстоянию Хаусдорфа

// площади пересечения элементов сетки и овала Кассини (size = Net[0].size())
vector<double> Areas;
vector<double> CoveredsAreas;

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

		// рисуем овал Кассини
		if (EnableDrawOval.GetCheck() == 1) {
			dc.SelectObject(redpen);
			if (RotatedAndMovedOvalKassini[0].size() > 0)
			{
				StructureForDrawPath = make_structure_for_draw(StructureForDrawPath, RotatedAndMovedOvalKassini);
				dc.Polyline(StructureForDrawPath, RotatedAndMovedOvalKassini[0].size());
			}
		}

		// рисуем ближайшую точку
		if (EnableDrawHausDot.GetCheck() == 1) {
			dc.SelectObject(redpen);
			if (Net[0].size() > 0) {
				dc.Ellipse(window_center_x + (scale * XHaus / scale_helper) - 3,
					window_center_y - (scale * YHaus / scale_helper) - 3,
					window_center_x + (scale * XHaus / scale_helper) + 3,
					window_center_y - (scale * YHaus / scale_helper) + 3);

				// и само расстояние Хаусдорфа
				dc.MoveTo(window_center_x + (scale * XHaus / scale_helper),
					window_center_y - (scale * YHaus / scale_helper));
				dc.LineTo(window_center_x + (scale * XOvalHaus / scale_helper),
					window_center_y - (scale * YOvalHaus / scale_helper));
			}
		}

		// рисуем точки, для которых считаем расстояние хаусдорфа
		if (EnableDrawDots.GetCheck() == 1) {
			dc.SelectObject(redpen);
			if (AllDots[0].size() > 0) {
				for (int i = 0; i < AllDots[0].size(); i++) {
					dc.Ellipse(window_center_x + (scale * AllDots[0][i][0].X / scale_helper) - 3,
						window_center_y - (scale * AllDots[0][i][0].Y / scale_helper) - 3,
						window_center_x + (scale * AllDots[0][i][0].X / scale_helper) + 3,
						window_center_y - (scale * AllDots[0][i][0].Y / scale_helper) + 3);
					//Sleep(500);
				}
			}
		}
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CStripsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//int scale_slider = 20; // так как слайдер принимает только целые занчения

double x_current = 0; // текущие значения точек в заполнении различных массивов
double y_current = 0; // текущие значения точек в заполнении различных массивов

// создаем овал Кассини
double a = 1.002 * scale_2; // параметры овала Кассини. "Играемся" масштабом
double c = 1 * scale_2;
double xC = sqrt(a * a + c * c);
double yA = (a * a) / (2 * c);

int N = 2 * ceil(xC);

double XOvalMax = 0; // крайние точки СМЕЩЕННОГО овала Кассини. Для построения оптимальной сетки
double YOvalMax = 0; // крайние точки СМЕЩЕННОГО овала Кассини. Для построения оптимальной сетки

double XOvalMin = 0; // крайние точки СМЕЩЕННОГО овала Кассини. Для построения оптимальной сетки
double YOvalMin = 0; // крайние точки СМЕЩЕННОГО овала Кассини. Для построения оптимальной сетки

int SliderCoreff = 1; // коэффициент для настройки диапазона слайдера. 1 - для кругов, 2 - дял квадратов

int NumOfBlocks = 2 * ceil(xC) * 2 * ceil(yA);

void CStripsDlg::add_oval()
{
	XOvalMin = 0;
	YOvalMin = 0;

	XOvalMax = 0;
	YOvalMax = 0;

	for (x_current = -xC + 0.001; x_current <= xC; x_current += 0.1) {
		y_current = sqrt(sqrt(a * a * a * a + 4 * c * c * x_current * x_current) - x_current * x_current - c * c);

		if (y_current >= 0) {
			OvalKassini[0] << IntPoint(x_current * scale_helper, y_current * scale_helper);
		}

		if (x_current > XOvalMax) {
			XOvalMax = x_current;
		}

		if (y_current > YOvalMax) {
			YOvalMax = y_current;
		}

		if (x_current < XOvalMin) {
			XOvalMin = x_current;
		}

		if (y_current < YOvalMin) {
			YOvalMin = y_current;
		}

	}

	// добавим самую крайнюю точку, чтобы овал не срезался
	x_current = xC - 0.001;
	y_current = sqrt(sqrt(a * a * a * a + 4 * c * c * x_current * x_current) - x_current * x_current - c * c);

	if (x_current > XOvalMax) {
		XOvalMax = x_current;
	}

	if (y_current > YOvalMax) {
		YOvalMax = y_current;
	}

	if (x_current < XOvalMin) {
		XOvalMin = x_current;
	}

	if (y_current < YOvalMin) {
		YOvalMin = y_current;
	}

	OvalKassini[0] << IntPoint(x_current * scale_helper, y_current * scale_helper);

	// рисуем нижнюю часть овала Кассини
	for (int i = OvalKassini[0].size() - 1; i >= 0; i--) {
		OvalKassini[0] << IntPoint(OvalKassini[0][i].X, -1 * OvalKassini[0][i].Y);
	}

	rotate_and_move_oval();

	// TODO: Add your control notification handler code here
}

void CStripsDlg::rotate_and_move_oval()
{
	// смещаем и поворачиваем (оператор поворота: x' = x*cos(alpha) - y*sin(alpha); y'= x*sin(alpha) + y*cos(alpha))

	RotatedAndMovedOvalKassini[0].clear();

	for (int i = 0; i < OvalKassini[0].size(); i++)
	{
		x_current = (OvalKassini[0][i].X * cos(AngleRotation.GetPos()*(PI / 180)) - OvalKassini[0][i].Y * sin(AngleRotation.GetPos()*(PI / 180))) +
			(XPosition.GetPos() * scale_helper / (SliderCoreff * SliderLimit));

		y_current = (OvalKassini[0][i].X * sin(AngleRotation.GetPos()*(PI / 180)) + OvalKassini[0][i].Y * cos(AngleRotation.GetPos()*(PI / 180))) +
			(YPosition.GetPos() * scale_helper / (SliderCoreff * SliderLimit));

		RotatedAndMovedOvalKassini[0] << IntPoint(x_current, y_current);
	}

	TextForCtrl.Format(_T("%.3f"), double(XPosition.GetPos()) / (SliderCoreff * SliderLimit));
	CurrXPos.SetWindowTextW(TextForCtrl);

	TextForCtrl.Format(_T("%.3f"), double(YPosition.GetPos()) / (SliderCoreff * SliderLimit));
	CurrYPos.SetWindowTextW(TextForCtrl);

	TextForCtrl.Format(_T("%d"), AngleRotation.GetPos());
	CurrAnglePos.SetWindowTextW(TextForCtrl);
}

// создаем сетку

double init_x = 0;
double init_y = 0;
double end_x = 0;
double end_y = 0;

double step_x = 0;
double step_y = 0;

double LimitBorder = 0; // для ОДНОКРАТНОГО создания заранее покрывающей сетки для всех позиций и поворотов

std::vector<Path> BlockRotated(1);

void CStripsDlg::add_net()
{
	Net[0].clear();
	Areas.clear();

	Intersections[0].clear();

	switch (NetType.GetCurSel())
	{
	case 0: {
		w = l;
		h = l;
	}; break; // для квадратов (по умолчанию 1 х 1)
	case 1: {
		w = l * 3;
		h = l * sqrt(3) / 2;
	}; break; // шестиугольник с длиной стороны l (по умолчанию l = 1)
	case 2: {
		w = l * 3;
		h = l * sqrt(3) / 2;
	}; break; // описанные круги вокруг шестиугольников
	case 3: {
		w = sqrt(2);
		h = sqrt(2);
	}; break; // описанные круги вокруг квадратов
	}

	LimitBorder = max(abs(XOvalMax), abs(YOvalMax));
	end_x = 0;
	end_y = 0;
	while (end_x <= LimitBorder + w)
	{
		end_x += w;
	}
	while (end_y <= LimitBorder + h)
	{
		end_y += h;
	}
	init_x = -1 * end_x;
	init_y = -1 * end_y;

	step_x = w;
	step_y = h;

	// координаты начала отрисовки текущего блока
	x_current = init_x; // используем те же промежуточные переменные current
	y_current = init_y;

	int LineCounter = 0;

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
	}; break; // для квадратов
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
					x_current = init_x + 1.5 * l;
				}
				else
				{
					x_current = init_x;
				}
				y_current += step_y;
			}
		}
	}; break;
	case 2: {
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
					x_current = init_x + 1.5 * l;
				}
				else
				{
					x_current = init_x;
				}
				y_current += step_y;
			}
		}
	}; break; // для шестиугольников
	case 3: {
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
	}; break;
	};

	//rotate_and_move_net(0, 0, 0);

	// TODO: Add your control notification handler code here
}

void CStripsDlg::rotate_and_move_net(double Angle, double ref_x, double ref_y)
{
	for (int i = 0; i < Net[0].size(); i++) {
		BlockRotated[0].clear();
		for (int j = 0; j < Net[0][i].size(); j++) {
			BlockRotated[0] << IntPoint((Net[0][i][j].X * cos(Angle * (PI / 180)) - Net[0][i][j].Y * sin(Angle * (PI / 180))) +
				((-SliderLimit * ref_x) * scale_helper / (SliderCoreff * SliderLimit)),
				(Net[0][i][j].X * sin(Angle * (PI / 180)) + Net[0][i][j].Y * cos(Angle * (PI / 180))) +
				((-SliderLimit * ref_y) * scale_helper / (SliderCoreff * SliderLimit)));
		}
		NetRotated[0].push_back(BlockRotated[0]);

		Net[0][i] = NetRotated[0][i];
		Net[0][i] = NetRotated[0][i];
	}
}

// создание отдельного блока для сетки
void CStripsDlg::add_block(double init_value_x, double init_value_y, double step_value_x, double step_value_y)
{
	Block[0].clear();

	switch (NetType.GetCurSel())
	{
	case 0: {
		// по часовой стрелке с левого верхнего угла

		// альтернативный вариант
		//for (int i = 45; i < 360; i += 90) {
		//	Block[0] << IntPoint((init_value_x + (sqrt(2)/2) * cos(i*PI / 180))*scale_helper, (init_value_y + (sqrt(2) / 2) * sin(i*PI / 180))*scale_helper);
		//}

		// верхняя линия
		Block[0] << IntPoint((init_value_x - 0.5) * scale_helper, (init_value_y - 0.5) * scale_helper);
		for (int i = 1; i < DotsOnEdge + 1; i++) {
			Block[0] << IntPoint((init_value_x + (1 / (double(DotsOnEdge) + 1)) * i)* scale_helper, init_value_y * scale_helper);
		}

		// правая вертикальная линия
		Block[0] << IntPoint((init_value_x + 0.5) * scale_helper, (init_value_y - 0.5) * scale_helper);
		for (int i = 1; i < DotsOnEdge + 1; i++) {
			Block[0] << IntPoint((init_value_x + step_value_y) * scale_helper, (init_value_y - (1 / (double(DotsOnEdge) + 1)) * i) * scale_helper);
		}

		// нижняя линия
		Block[0] << IntPoint((init_value_x + 0.5) * scale_helper, (init_value_y + 0.5) * scale_helper);
		for (int i = 1; i < DotsOnEdge + 1; i++) {
			Block[0] << IntPoint((init_value_x + step_value_x - (1 / (double(DotsOnEdge) + 1)) * i) * scale_helper, (init_value_y - step_value_y) * scale_helper);
		}

		// левая линия
		Block[0] << IntPoint((init_value_x - 0.5) * scale_helper, (init_value_y + 0.5) * scale_helper);
		for (int i = 1; i < DotsOnEdge + 1; i++) {
			Block[0] << IntPoint(init_value_x  * scale_helper, (init_value_y - step_value_y + (1 / (double(DotsOnEdge) + 1)) * i) * scale_helper);
		}
	}; break; // для квадратов
	case 1: {
		for (int i = 0; i < 360; i += 60) {
			Block[0] << IntPoint((init_value_x + l * cos(i*PI / 180))*scale_helper, (init_value_y + l * sin(i*PI / 180))*scale_helper);
		}
	}; break; // для шестиугольников
	case 2: {
		for (int i = 0; i < 360; i += 1) {
			Block[0] << IntPoint((init_value_x + l * cos(i*PI / 180))*scale_helper, (init_value_y + l * sin(i*PI / 180))*scale_helper);
		}
	}; break; // для кругов
	case 3: {
		for (int i = 0; i < 360; i += 2) {
			Block[0] << IntPoint((init_value_x + 0 / double(2) + cos(i * PI / 180)) * scale_helper, (init_value_y + 0 / double(2) + sin(i * PI / 180)) * scale_helper);
		}
	}; break;
	};

	Net[0].push_back(Block[0]);
}

// пересекаем овал Кассини и сетку
void CStripsDlg::add_intersec_oval_and_net()
{
	Intersections[0].clear();
	CoveredNet[0].clear();
	CoveredsAreas.clear();

	for (int i = 0; i < Net[0].size(); i++)
	{
		CurrentIntersection = do_intersectrion(Net, i, RotatedAndMovedOvalKassini);
		if (CurrentIntersection.size() > 0)
		{
			Intersections[0].push_back(CurrentIntersection[0]);
			Areas.push_back(Area(CurrentIntersection[0]) / pow(scale_helper, 2));
			CoveredsAreas.push_back(Area(CurrentIntersection[0]) / pow(scale_helper, 2));
			CoveredNet[0].push_back(Net[0][i]);
		}
		else
		{
			Areas.push_back(0);
		}
	}
	TextForCtrl.Format(_T("%d"), CoveredNet[0].size());
	NumOfCoveredBlocks.SetWindowTextW(TextForCtrl);

	// TODO: Add your control notification handler code here
}

double HausDist = 0; // указываем расстояние, которое точно меньше минимального
double CurrHausDist = 0;

bool FoundOnAnEdge = false;
int CountOfAnEdge = 0;

std::vector<Path> CheckedDots(1);

double BlockArea = 0;

// считаем Хаусдорфово расстояние
void CStripsDlg::find_haus_dist()
{
	BlockArea = abs(Area(Net[0][0])) / (scale_helper * scale_helper); // площадь одной клетки сетки

	CheckedDots[0].clear();
	AllDots[0].clear();

	// для начала надо определить, лежит ли точка блока сетки в овале Кассини. Для этого преобразуем точки в маааленький треугольник
	// и пересечем с овалом Кассини

	// здесь мы теряем привязанность точек к конкретному блоку, но она нам и не сильно нужна.
	
	//for (int i = 0; i < Areas.size(); i++) { // i - это блоки сетки
	for (int i = 0; i < CoveredNet[0].size(); i++) { // i - это блоки сетки
		if (int(CoveredsAreas[i] * 100) != int(BlockArea * 100)) // 0 - не пересекается, BlockArea - лежит внутри овала Кассини
																 // аккуратно округляем до двух знаков после запятой
																 // а то площадь круга везде разная!
		{
			DotsOfCurrBlock = make_polys_for_dots(CoveredNet[0], i, CheckedDots); // превратили точки в маааленькие треугольники

			// проверяем, лежит ли маааленький треугольик в овале Кассини. Если лежит, то мы его игнорируем
			for (int j = 0; j < DotsOfCurrBlock[0].size(); j++) { // j - это точки частично покрывающего блока
				CurrentIntersection = do_intersectrion(DotsOfCurrBlock, j, RotatedAndMovedOvalKassini);

				if (CurrentIntersection.size() == 0 || ((CurrentIntersection.size() > 0) && // если мааленький лежит снаружи
						(abs(Area(CurrentIntersection[0])) < abs(Area(DotsOfCurrBlock[0][0]))))) { // или касается частично овала Кассини (что бывает ооочень редко)

					// добавим точку в реестр проверяемых точек
					// воспользуемся тем, что самая первая точка в мааленьком треугольнике точно сверху
					CheckedDots[0] << IntPoint(DotsOfCurrBlock[0][j][0].X, DotsOfCurrBlock[0][j][0].Y + (0.01 * scale_helper));

					AllDots[0].push_back(DotsOfCurrBlock[0][j]);

					CurrHausDist = count_nearest_distance(DotsOfCurrBlock, j, 0);

					if (CurrHausDist > HausDist) {
						HausDist = CurrHausDist; // узнали расстояние
						XHaus = DotsOfCurrBlock[0][j][0].X;
						YHaus = DotsOfCurrBlock[0][j][0].Y + (0.01 * scale_helper);

						XOvalHaus = XOvalHausPrev;
						YOvalHaus = YOvalHausPrev;
					}
				}
			}
		}
	}
	HausDist /= scale_helper;

	//if ((XHaus % 1000 != 0) || (YHaus % 1000 != 0)) {
	//	CountOfAnEdge++;
	//	if (CountOfAnEdge == 1) {
	//		FoundOnAnEdge = true;
	//	}
	//}

	TextForCtrl.Format(_T("%.4f"), (HausDist / scale_2) * 5);
	HausdorffDistance.SetWindowTextW(TextForCtrl);

	// TODO: Add your control notification handler code here
}


// ищем минмальное Хаусдорфово расстояние
void CStripsDlg::do_check_all_positions()
{
	do_ints.SetCheck(1);

	double MinHausDist = 20;
	int count = 0;

	NumOfBlocks = 2 * ceil(xC) * 2 * ceil(yA);

	// зафиксируем, в каком случае достигается минимальное значение Хаусдорфова расстояния
	int MinXPos = 0;
	int MinYPos = 0;
	int MinAnglePos = 0;

	// для однократного формирования сетки
	add_oval();
	add_net();

	for (int i = XPosition.GetRangeMin(); i <= XPosition.GetRangeMax(); i++) {
		/*if (FoundOnAnEdge == true) {
			break;
		}*/
		XPosition.SetPos(i); // один фиг: он не заполнит edit control-ы, пока циклы не закончатся
		for (int j = YPosition.GetRangeMin(); j <= YPosition.GetRangeMax(); j++) {
	/*		if (FoundOnAnEdge == true) {
				break;
			}*/
			YPosition.SetPos(j);
			//if (sqrt(i * i + j * j) > SliderLimit)
			//{
			//	continue;
			//}
			for (int k = AngleRotation.GetRangeMin(); k <= 90; k += 1) {
				//if (FoundOnAnEdge == true) {
				//	MinXPos = XPosition.GetPos();
				//	MinYPos = YPosition.GetPos();
				//	MinAnglePos = AngleRotation.GetPos();
				//	break;
				//}

				AngleRotation.SetPos(k);

				rotate_and_move_oval();
				add_intersec_oval_and_net();

				switch (Regime.GetCurSel()) { // проверяем, что мы сейчас считаем
				case 0: {
					find_haus_dist();

					//draw_everything();

					if (HausDist < MinHausDist) {
						MinHausDist = HausDist;

						MinXPos = XPosition.GetPos();
						MinYPos = YPosition.GetPos();
						MinAnglePos = AngleRotation.GetPos();
					}
				}; break;
				case 1:
				{
					//draw_everything();

					if (Net[0].size() < NumOfBlocks) {
						NumOfBlocks = Net[0].size();

						MinXPos = XPosition.GetPos();
						MinYPos = YPosition.GetPos();
						MinAnglePos = AngleRotation.GetPos();
					}
				};
				}
			}
		}
	}

	// установим подходящие условия
	set_slider(XPosition, MinXPos, 1, CurrXPos);
	set_slider(YPosition, MinYPos, 1, CurrYPos);
	set_slider(AngleRotation, MinAnglePos, 1, CurrAnglePos);

	rotate_and_move_oval();
	//add_net();
	add_intersec_oval_and_net();
	find_haus_dist();

	TextForCtrl.Format(_T("%.4f"), (MinHausDist / scale_2) * 5);
	HausdorffDistance.SetWindowTextW(TextForCtrl);

	TextForCtrl.Format(_T("%d"), CoveredNet[0].size());
	NumOfCoveredBlocks.SetWindowTextW(TextForCtrl);

	draw_everything();

	// TODO: Add your control notification handler code here
}

// ищем кратчайшее расстояния для каждой вершины покрывающего блока сетки
double MinDistance = 20 * scale_helper; // указываем расстяние, которое точно больше максимального
double CurrentDistance = 20 * scale_helper;

double CStripsDlg::count_nearest_distance(std::vector<Paths> net_block, int num_of_block, int num_of_dot)
{
	MinDistance = 20 * scale_helper;
	CurrentDistance = 20 * scale_helper;

	for (int i = 0; i < RotatedAndMovedOvalKassini[0].size(); i++) { // i - точки оала Кассини
		CurrentDistance = sqrt(pow((net_block[0][num_of_block][num_of_dot].X - RotatedAndMovedOvalKassini[0][i].X), 2) +
			pow((net_block[0][num_of_block][num_of_dot].Y + (0.01 * scale_helper) - RotatedAndMovedOvalKassini[0][i].Y), 2));

		if (CurrentDistance < MinDistance) {
			MinDistance = CurrentDistance;

			XOvalHausPrev = RotatedAndMovedOvalKassini[0][i].X;
			YOvalHausPrev = RotatedAndMovedOvalKassini[0][i].Y;
		}
	}

	return MinDistance;
}

// общая функция пересечения
std::vector<Path> CStripsDlg::do_intersectrion(std::vector<Paths> who_clip, int num_of_path, std::vector<Path> who_clipped)
{
	cp.Clear();
	CurrInts.clear();

	cp.AddPath(who_clip[0][num_of_path], ptSubject, true);
	cp.AddPath(who_clipped[0], ptClip, true);
	cp.Execute(ctIntersection, CurrInts, pftNonZero, pftNonZero);

	return CurrInts;
}

std::vector<Path> CStripsDlg::do_intersectrion(std::vector<Path> who_clip, std::vector<Path> who_clipped)
{

	cp.Clear();
	CurrInts.clear();

	cp.AddPath(who_clip[0], ptSubject, true);
	cp.AddPath(who_clipped[0], ptClip, true);
	cp.Execute(ctIntersection, CurrInts, pftNonZero, pftNonZero);

	return CurrInts;
}

double CStripsDlg::check_for_empty_ints(std::vector<Path> who_clip, std::vector<Path> who_clipped)
{	
	if (do_intersectrion(who_clip, who_clipped).size() > 0)
	{
		return Area(do_intersectrion(who_clip, who_clipped)[0]);
	}
	else
	{
		return 0;
	}
}

// реагируем на движение слайдера
void CStripsDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CSliderCtrl *pSlider = reinterpret_cast<CSliderCtrl*>(pScrollBar);

	if (pSlider == &XPosition || pSlider == &YPosition || pSlider == &AngleRotation || pSlider == &ScaleOfOval) {
		add_intersec_oval_and_net();
		rotate_and_move_oval();
	}

	draw_everything();

	// TODO: Add your message handler code here and/or call default

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}

// рисуем все, что создали
void CStripsDlg::draw_everything()
{
	InvalidateRect(RedrawArea, 1);
	OnPaint();

	// TODO: Add your control notification handler code here
}

// настраиваем слайдер и поле начения для него
void CStripsDlg::set_slider(CSliderCtrl& slider, int position, int divide_position, CEdit& value)
{
	slider.SetPos(position);

	TextForCtrl.Format(_T("%.2f"), double(position) / divide_position);
	value.SetWindowTextW(TextForCtrl);
}


void CStripsDlg::OnBnClickedCheck2()
{
	draw_everything();
	// TODO: Add your control notification handler code here
}


void CStripsDlg::OnBnClickedCheck3()
{
	draw_everything();
	// TODO: Add your control notification handler code here
}


void CStripsDlg::OnBnClickedCheck4()
{
	draw_everything();
	// TODO: Add your control notification handler code here
}


void CStripsDlg::OnBnClickedCheck5()
{
	draw_everything();
	// TODO: Add your control notification handler code here
}


void CStripsDlg::OnBnClickedCheck6()
{
	draw_everything();
	// TODO: Add your control notification handler code here
}
