
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

double w = 1; // параметры блоков сетки
double h = w;

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

		XPosition.SetRange(-w * SliderLimit, w * SliderLimit, 1);
		XPosition.SetPos(0);

		YPosition.SetRange(-w * SliderLimit, w * SliderLimit, 1);
		YPosition.SetPos(0);

		AngleRotation.SetRange(0, 360, 1);
		AngleRotation.SetPos(0);

		Regime.AddString(_T("1. Хаусдорфово расстояние"));
		Regime.AddString(_T("2. Число блоков"));
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

std::vector<Paths> Net(1);
std::vector<Path> OvalKassini(1); // изначальный в начале координат. На его основании поворачиваем и двигаем
std::vector<Path> RotatedAndMovedOvalKassini(1);
std::vector<Paths> Intersections(1);
std::vector<Paths> DotsOfCurrBlock(1);

std::vector<Paths> AllDots(1);

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
		dc.SelectObject(blackpen);
		if (Net[0].size() > 0)
		{
			StructureForDrawPaths = make_structure_for_draw(StructureForDrawPaths, Net);
			for (int i = 0; i < Net[0].size(); i++) {
				dc.Polygon(StructureForDrawPaths[i], Net[0][i].size());
			}
		}

		// рисуем овал Кассини
		dc.SelectObject(redpen);
		if (RotatedAndMovedOvalKassini[0].size() > 0)
		{
			StructureForDrawPath = make_structure_for_draw(StructureForDrawPath, RotatedAndMovedOvalKassini);
			dc.Polyline(StructureForDrawPath, RotatedAndMovedOvalKassini[0].size());
		}

		// рисуем пересечение
		dc.SelectObject(bluepen);
		if (Intersections[0].size() > 0)
		{
			StructureForDrawPaths = make_structure_for_draw(StructureForDrawPaths, Intersections);
			for (int i = 0; i < Intersections[0].size(); i++) {
				dc.Polygon(StructureForDrawPaths[i], Intersections[0][i].size());
			}
		}

		// рисуем ближайшую точку
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

		// рисуем точки, для которых считаем расстояние хаусдорфа
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
double a = 1.1 * scale_2; // параметры овала Кассини. "Играемся" масштабом
double c = 1 * scale_2;
double xC = sqrt(a * a + c * c);
double yA = (a * a) / (2 * c);

int N = 2 * ceil(xC);

double XOvalMax = 0; // крайние точки СМЕЩЕННОГО овала Кассини. Для построения оптимальной сетки
double YOvalMax = 0; // крайние точки СМЕЩЕННОГО овала Кассини. Для построения оптимальной сетки

double XOvalMin = 0; // крайние точки СМЕЩЕННОГО овала Кассини. Для построения оптимальной сетки
double YOvalMin = 0; // крайние точки СМЕЩЕННОГО овала Кассини. Для построения оптимальной сетки


void CStripsDlg::add_oval()
{
	if (OvalKassini[0].size() == 0) {
		for (x_current = -xC + 0.001; x_current <= xC; x_current += 0.1) {
			y_current = sqrt(sqrt(a * a * a * a + 4 * c * c * x_current * x_current) - x_current * x_current - c * c);

			if (y_current >= 0) {
				OvalKassini[0] << IntPoint(x_current * scale_helper, y_current * scale_helper);
			}
		}

		// добавим самую крайнюю точку, чтобы овал не срезался
		x_current = xC - 0.001;
		y_current = sqrt(sqrt(a * a * a * a + 4 * c * c * x_current * x_current) - x_current * x_current - c * c);

		OvalKassini[0] << IntPoint(x_current * scale_helper, y_current * scale_helper);

		// рисуем нижнюю часть овала Кассини
		for (int i = OvalKassini[0].size() - 1; i >= 0; i--) {
			OvalKassini[0] << IntPoint(OvalKassini[0][i].X, -1 * OvalKassini[0][i].Y);
		}
	}

	// смещаем и поворачиваем (оператор поворота: x' = x*cos(alpha) - y*sin(alpha); y'= x*sin(alpha) + y*cos(alpha))

	RotatedAndMovedOvalKassini[0].clear();

	XOvalMax = 0;
	YOvalMax = 0;
	XOvalMin = 0;
	YOvalMin = 0;

	for (int i = 0; i < OvalKassini[0].size(); i++) {
		x_current = (OvalKassini[0][i].X * cos(AngleRotation.GetPos()*(PI / 180)) - OvalKassini[0][i].Y * sin(AngleRotation.GetPos()*(PI / 180))) +
			(XPosition.GetPos() * scale_helper / (2 * SliderLimit));

		y_current = (OvalKassini[0][i].X * sin(AngleRotation.GetPos()*(PI / 180)) + OvalKassini[0][i].Y * cos(AngleRotation.GetPos()*(PI / 180))) +
			(YPosition.GetPos() * scale_helper / (2 * SliderLimit));

		RotatedAndMovedOvalKassini[0] << IntPoint(x_current, y_current);

		if (x_current > XOvalMax) { // знак "меньше" потому что значения меньше нуля
			XOvalMax = x_current;
		}

		if (y_current > YOvalMax) {
			YOvalMax = y_current;
		}

		if (x_current < XOvalMin) { // знак "меньше" потому что значения меньше нуля
			XOvalMin = x_current;
		}

		if (y_current < YOvalMin) {
			YOvalMin = y_current;
		}
	}

	TextForCtrl.Format(_T("%.2f"), double(XPosition.GetPos()) / (2 * SliderLimit));
	CurrXPos.SetWindowTextW(TextForCtrl);

	TextForCtrl.Format(_T("%.2f"), double(YPosition.GetPos()) / (2 * SliderLimit));
	CurrYPos.SetWindowTextW(TextForCtrl);

	TextForCtrl.Format(_T("%d"), AngleRotation.GetPos());
	CurrAnglePos.SetWindowTextW(TextForCtrl);

	// TODO: Add your control notification handler code here
}

// создаем сетку

double init_x = 0; // -8 - 1
double init_y = 0; // 4 * 2 + 1
double end_x = 0; // -8 - 1
double end_y = 0; // 4 * 2 + 1

double step_x = w;
double step_y = h;

void CStripsDlg::add_net()
{
	Net[0].clear();

	// начинаем рисовать сетку из верхнего левого угла (двигаемся вправо и вниз). Начало координат в середине окна

	init_x = -1 * ceil(abs(XOvalMin) / scale_helper); // floor так как, опять же, меньше нуля
	init_y = ceil(abs(YOvalMax) / scale_helper);

	end_x = ceil(XOvalMax / scale_helper); // floor так как, опять же, меньше нуля
	end_y = -1 * ceil(abs(YOvalMin) / scale_helper);

	//double t = (YPosition.GetPos() / 4);

	// координаты начала отрисовки текущего блока
	x_current = init_x; // используем те же промежуточные переменные current
	y_current = init_y;

	while (y_current > end_y)
	{
		add_block(x_current, y_current, step_x, step_y);
		x_current += step_x;
		if (x_current > end_x - 1)
		{
			x_current = init_x;
			y_current -= step_y;
		}
	}

	int t = 0;

	// TODO: Add your control notification handler code here
}

std::vector<Path> Block(1);

int DotsOnEdge = 5; // число точек на грани блкоа сетки

// создание отдельного блока для сетки
void CStripsDlg::add_block(double init_value_x, double init_value_y, double step_value_x, double step_value_y)
{
	Block[0].clear();

	// по часовой стрелке с левого верхнего угла
	// верхняя линия
	Block[0] << IntPoint(init_value_x * scale_helper, init_value_y * scale_helper);
	for (int i = 1; i < DotsOnEdge + 1; i++) {
		Block[0] << IntPoint((init_value_x + (1 / (double(DotsOnEdge) + 1)) * i)* scale_helper, init_value_y * scale_helper);
	}

	// правая вертикальная линия
	Block[0] << IntPoint((init_value_x + step_value_x) * scale_helper, init_value_y * scale_helper);
	for (int i = 1; i < DotsOnEdge + 1; i++) {
		Block[0] << IntPoint((init_value_x + step_value_y) * scale_helper, (init_value_y - (1 / (double(DotsOnEdge) + 1)) * i) * scale_helper);
	}

	// нижняя линия
	Block[0] << IntPoint((init_value_x + step_value_x) * scale_helper, (init_value_y - step_value_y) * scale_helper);
	for (int i = 1; i < DotsOnEdge + 1; i++) {
		Block[0] << IntPoint((init_value_x + step_value_x - (1 / (double(DotsOnEdge) + 1)) * i) * scale_helper, (init_value_y - step_value_y) * scale_helper);
	}

	// левая линия
	Block[0] << IntPoint(init_value_x * scale_helper, (init_value_y - step_value_y) * scale_helper);
	for (int i = 1; i < DotsOnEdge + 1; i++) {
		Block[0] << IntPoint(init_value_x  * scale_helper, (init_value_y - step_value_y + (1 / (double(DotsOnEdge) + 1)) * i) * scale_helper);
	}

	Net[0].push_back(Block[0]);
}

// площади пересечения элементов сетки и овала Кассини (size = Net[0].size())
vector<double> Areas;

int CoveredBlocksCounter = 0; // считаем, сколько блоков покрывают частично или полностью овал Кассини

// пересекаем овал Кассини и сетку
void CStripsDlg::add_intersec_oval_and_net()
{
	Areas.clear();
	CoveredBlocksCounter = 0;

	Intersections[0].clear();

	for (int i = 0; i < Net[0].size(); i++) { // i - отдельный блок сетки 
		CurrentIntersection = do_intersectrion(Net, i, RotatedAndMovedOvalKassini); // пересекли

		// проверяем результат пересечения
		if (CurrentIntersection.size() > 0) {
			CoveredBlocksCounter++;
			Intersections[0].push_back(CurrentIntersection[0]);
			Areas.push_back(Area(CurrentIntersection[0]) / pow(scale_helper, 2));
		}
		else {
			Areas.push_back(0);
		}
	}

	TextForCtrl.Format(_T("%d"), CoveredBlocksCounter);
	NumOfCoveredBlocks.SetWindowTextW(TextForCtrl);

	// TODO: Add your control notification handler code here
}

double HausDist = 0; // указываем расстояние, которое точно меньше минимального
double CurrHausDist = 0;

bool FoundOnAnEdge = false;
int CountOfAnEdge = 0;

std::vector<Path> CheckedDots(1);

// считаем Хаусдорфово расстояние
void CStripsDlg::find_haus_dist()
{
	double BlockArea = abs(Area(Net[0][0])) / (scale_helper * scale_helper); // площадь одной клетки сетки

	CheckedDots[0].clear();
	AllDots[0].clear();

	// для начала надо определить, лежит ли угол блока в овале Кассини. Для этого преобразуем точки в маааленький треугольник
	// и пересечем с овалом Кассини

	// здесь мы теряем привязанность точек к конкретному блоку, но она нам и не сильно нужна.

	for (int i = 0; i < Areas.size(); i++) { // i - это блоки сетки
		if (i == 74) {
			int t = 0;
		}
		if ((Areas[i] != 0) && (Areas[i] != BlockArea)) { // 0 - не пересекается, BlockArea - лежит внутри овала Кассини

			DotsOfCurrBlock = make_polys_for_dots(Net[0], i, CheckedDots); // превратили точки в маааленькие треугольники

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

	if ((XHaus % 1000 != 0) || (YHaus % 1000 != 0)) {
		CountOfAnEdge++;
		if (CountOfAnEdge == 1) {
			FoundOnAnEdge = true;
		}
	}

	TextForCtrl.Format(_T("%.2f"), (HausDist / scale_2) * 5);
	HausdorffDistance.SetWindowTextW(TextForCtrl);

	// TODO: Add your control notification handler code here
}

// ищем минмальное Хаусдорфово расстояние
void CStripsDlg::do_check_all_positions()
{
	double MinHausDist = 20;
	int count = 0;

	int NumOfBlocks = 2 * ceil(xC) * 2 * ceil(yA);

	// зафиксируем, в каком случае достигается минимальное значение Хаусдорфова расстояния
	int MinXPos = 0;
	int MinYPos = 0;
	int MinAnglePos = 0;

	for (int i = XPosition.GetRangeMin(); i <= XPosition.GetRangeMax(); i++) {
		if (FoundOnAnEdge == true) {
			break;
		}
		XPosition.SetPos(i); // один фиг: он не заполнит edit control-ы, пока циклы не закончатся
		for (int j = YPosition.GetRangeMin(); j <= YPosition.GetRangeMax(); j++) {
			if (FoundOnAnEdge == true) {
				break;
			}
			YPosition.SetPos(j);
			for (int k = AngleRotation.GetRangeMin(); k <= 90; k += 1) {
				if (FoundOnAnEdge == true) {
					MinXPos = XPosition.GetPos();
					MinYPos = YPosition.GetPos();
					MinAnglePos = AngleRotation.GetPos();
					break;
				}

				AngleRotation.SetPos(k);

				add_oval();
				add_net();
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
					if (CoveredBlocksCounter < NumOfBlocks) {
						NumOfBlocks = CoveredBlocksCounter;

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

	add_oval();
	add_net();
	add_intersec_oval_and_net();
	find_haus_dist();

	TextForCtrl.Format(_T("%.2f"), (MinHausDist / scale_2) * 5);
	HausdorffDistance.SetWindowTextW(TextForCtrl);

	TextForCtrl.Format(_T("%d"), NumOfBlocks);
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

	for (int i = 0; i < OvalKassini[0].size(); i++) { // i - точки оала Кассини
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

// реагируем на движение слайдера
void CStripsDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CSliderCtrl *pSlider = reinterpret_cast<CSliderCtrl*>(pScrollBar);

	if (pSlider == &XPosition || pSlider == &YPosition || pSlider == &AngleRotation || pSlider == &ScaleOfOval) {
		if (pSlider == &ScaleOfOval) {

			//scale_2 = ScaleOfOval.GetPos();
			//scale = scale_1 / scale_2;
			//a = 1.1 * scale_2;
			//c = 1 * scale_2;

			//RedrawArea.SetRect(window_center_x - 10 * scale, window_center_y - 10 * scale, window_center_x + 10 * scale, window_center_y + 10 * scale);
			//set_drawing_param(window_center_x, window_center_y, scale, scale_helper);

		}
		add_net();
		add_oval();
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
