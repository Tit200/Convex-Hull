#include "MainFrame.h"
#include<wx/wx.h>
#include <wx/spinctrl.h>
#include<random>
#include <wx/stopwatch.h>
#include<algorithm>
#include <unordered_set>
#include <functional>
#include <stack>
#include <cmath>


// v tem file-u dodajam funkcionalnosti GUI okencu
MainFrame::MainFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title)
{
	wxPanel* panel = new wxPanel(this);
	// Otrok MainFrame-a, vnos: (stars)

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);    // Za bottom panel
	wxBoxSizer* controlSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);   // Za kontrole in grid

	wxStaticText* static_text_algorithm = new wxStaticText(panel, wxID_ANY, "Algorithm");
	controlSizer->Add(static_text_algorithm, 0, wxALL, 5);
	// ustvari text 

	rbJarvis = new wxRadioButton(panel, wxID_ANY, wxT("Jarvis March"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	rbGraham = new wxRadioButton(panel, wxID_ANY, wxT("Graham Scan"));
	rbQuick = new wxRadioButton(panel, wxID_ANY, wxT("Quick Hull"));

	controlSizer->Add(rbJarvis, 0, wxALL, 5);
	controlSizer->Add(rbGraham, 0, wxALL, 5);
	controlSizer->Add(rbQuick, 0, wxALL, 5);

	wxStaticText* static_text_point_dist = new wxStaticText(panel, wxID_ANY, "Point distribution");
	controlSizer->Add(static_text_point_dist, 0, wxALL, 5);
	// ustvari text 

	rbNormal = new wxRadioButton(panel, wxID_ANY, wxT("Normal"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	rbUniform = new wxRadioButton(panel, wxID_ANY, wxT("Uniform"));

	controlSizer->Add(rbNormal, 0, wxALL, 5);
	controlSizer->Add(rbUniform, 0, wxALL, 5);


	wxStaticText* stNumOfPoints = new wxStaticText(panel, wxID_ANY, "Number of Points");
	controlSizer->Add(stNumOfPoints, 0, wxALL, 5);

	spinCtrl = new wxSpinCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(100, -1), wxSP_ARROW_KEYS, 0, 1000000000, 100000);
	controlSizer->Add(spinCtrl, 0, wxALL, 5);

	wxButton* bGeneratePoints = new wxButton(panel, wxID_ANY, "Generate Points",wxDefaultPosition, wxSize(100, 35));
	wxButton* bCalculateHull = new wxButton(panel, wxID_ANY, "Calculate Hull",wxDefaultPosition ,wxSize(100, 35));
	
	controlSizer->Add(bGeneratePoints, 0, wxALL, 5);
	controlSizer->Add(bCalculateHull, 0, wxALL, 5);

	bGeneratePoints->Bind(wxEVT_BUTTON, &MainFrame::OnGeneratePoints, this);
	bCalculateHull->Bind(wxEVT_BUTTON, &MainFrame::OnCalculateHull, this);

	// Ustvarim se panel za izris
	pointPanel = new PointPanel(panel);
	pointPanel->SetBackgroundColour(*wxWHITE);

	topSizer->Add(controlSizer, 0, wxEXPAND | wxALL, 10);
	topSizer->Add(pointPanel, 1, wxEXPAND | wxALL, 10);

	mainSizer->Add(topSizer, 1, wxEXPAND | wxALL, 10);
	
	// Ustvarim se info panel na dnu, za obvescanje
	infoTextCtrl = new wxTextCtrl(panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxVSCROLL | wxHSCROLL);
	mainSizer->Add(infoTextCtrl, 0, wxEXPAND | wxALL, 10);
	infoTextCtrl->SetMinSize(wxSize(-1, 100)); // osnovna velikost info box-a
 
	panel->SetSizer(mainSizer);

	CreateStatusBar(); //ustvari spodaj status_bar v okencu	
}

//-------------------------------------------------------
//Koda, ki glede na izbor radio Buttona klice normalno ali enakomerno porazdelitveno funkcijo
//-------------------------------------------------------
void MainFrame::OnGeneratePoints(wxCommandEvent& evt) 
{

	int numPoints = spinCtrl->GetValue();
	generatedPoints.clear(); // pocisti prejsnje generirane tocke
	std::vector<wxPoint> points;

	wxStopWatch stopwatch; // Zacne merit cas

	if (rbNormal->GetValue()) 
	{
		GenerateNormalPoints(numPoints, points);
	}
	
	else if (rbUniform->GetValue())
	{
		GenerateUniformPoints(numPoints, points);
	}
	else
	{
		wxLogError("Select a point distribution method.");
		return;
	}

	generatedPoints = points; // shranim generirane tocke, saj jih rabim za izvajanje algoritmov
	pointPanel->SetPoints(points);

	// Sporoci uporabniku
	infoTextCtrl->AppendText(wxString::Format("Points generated in %ld ms\n \n", stopwatch.Time()));
	infoTextCtrl->ShowPosition(infoTextCtrl->GetLastPosition());
}

//Koda za generacijo normalno porazdeljenih tock
void MainFrame::GenerateNormalPoints(int numPoints, std::vector<wxPoint>& points) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::normal_distribution<> distX(0, 50); // mean = (0,0) st.dev. = (50,50)
	std::normal_distribution<> distY(0, 50); 

	for (int i = 0; i < numPoints; ++i) {
		int x = std::round(distX(gen));
		int y = std::round(distY(gen));
		points.emplace_back(x, y);
	}
}

//Koda za generacijo enakomerne porazdelitve tock
void MainFrame::GenerateUniformPoints(int numPoints, std::vector<wxPoint>& points)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distX(-200, 200); 
	std::uniform_int_distribution<> distY(-200, 200); 

	for (int i = 0; i < numPoints; ++i) {
		int x = distX(gen);
		int y = distY(gen);
		points.emplace_back(x, y);
	}
}

// -------------------------------------------------------
// Koda, ki glede na izbiro rb izbere kateri algoritem se klice
// -------------------------------------------------------

void MainFrame::OnCalculateHull(wxCommandEvent& evt) 
{
	if (generatedPoints.empty()) 
	{
		wxLogError("No points generated.");
		return;
	}
	
	std::vector<wxPoint> hull;
	wxStopWatch stopwatch; // Zacne merit cas

	// izbere algoritem, glede na izbiro RadioButtona
	if (rbJarvis->GetValue()) 
	{
		hull = JarvisMarch(generatedPoints); 
		pointPanel->SetHull(hull);

		infoTextCtrl->AppendText(wxString::Format("Jarvis March executed in %ld ms\n \n", stopwatch.Time()));
		
	}
	else if (rbGraham->GetValue())
	{
		hull = GrahamScan(generatedPoints);
		pointPanel->SetHull(hull);

		infoTextCtrl->AppendText(wxString::Format("Graham Scan executed in %ld ms\n", stopwatch.Time()));
	}
	else if (rbQuick->GetValue())
	{
		hull = QuickHull(generatedPoints);
		pointPanel->SetHull(hull);

		infoTextCtrl->AppendText(wxString::Format("Quick Hull executed in %ld ms\n", stopwatch.Time()));
	}
	else
	{
		wxLogError("Error");
		return;
	}

	infoTextCtrl->ShowPosition(infoTextCtrl->GetLastPosition());
}
// ----------------------------------------
//	Algoritem: Jarvis March
// ----------------------------------------
std::vector<wxPoint> MainFrame::JarvisMarch(const std::vector<wxPoint>& points)
{
	std::vector<wxPoint> hull;  // prazen array: hull
	int n = points.size();      //stevilo tock: n

	if (n < 3)
		return hull;

	int min = 0;

	for (int i = 1; i < n; i++)
	{
		if (points[i].x < points[min].x)
			min = i;
	}

	int p = min, q;
	do
	{
		hull.push_back(points[p]);
		q = (p + 1) % n;
		for (int i = 0; i < n; i++)
		{
			double crossProduct = (points[i].x - points[p].x) * (points[q].y - points[p].y) - (points[i].y - points[p].y) * (points[q].x - points[p].x);
			if (crossProduct > 0)
				q = i;
		}
		p = q;
	} while (p != min);

	return hull;
}


// Skalarni produkt
double CrossProduct(const wxPoint& o, const wxPoint& a, const wxPoint& b) {
	return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

// ----------------------------------------
//	Algoritem: Grahams Scan
// ----------------------------------------

// Polarni kot med tockama O in A glede na x-os
double PolarAngle(const wxPoint& O, const wxPoint& A) {
	return std::atan2(A.y - O.y, A.x - O.x);
}

// Uredi tocke glede na kot
bool ComparePolarAngle(const wxPoint& O, const wxPoint& A, const wxPoint& B) {
	return PolarAngle(O, A) < PolarAngle(O, B);
}

// Definirana razdalja
double wxDist(const wxPoint& p1, const wxPoint& p2) {
	return std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
}

// Graham Scan algoritem
std::vector<wxPoint> MainFrame::GrahamScan(const std::vector<wxPoint>& points) {
	int n = points.size();
	if (n < 3) return {}; 

	// najdemo tocko z min. y-koordinato
	wxPoint start = *std::min_element(points.begin(), points.end(), [](const wxPoint& a, const wxPoint& b) {
		return (a.y < b.y) || (a.y == b.y && a.x < b.x);
		});

	// tocke sortiramo glede na kot in zacetno tocko
	std::vector<wxPoint> sortedPoints = points;
	std::sort(sortedPoints.begin(), sortedPoints.end(), [start](const wxPoint& a, const wxPoint& b) {
		double cross = CrossProduct(start, a, b);
		if (cross == 0) return wxDist(start, a) < wxDist(start, b); 
		return cross > 0; // sortiramo po skalarnem produktu kontra urinemu-kazalcu
		});

	// sestavimo hull
	std::vector<wxPoint> hull;
	hull.push_back(start);

	for (const wxPoint& point : sortedPoints) {
		while (hull.size() >= 2 && CrossProduct(hull[hull.size() - 2], hull.back(), point) <= 0) {
			hull.pop_back(); // Odstranimo tocke, ki ne zavijejo levo
		}
		hull.push_back(point);
	}

	return hull;
}

// ----------------------------------------
//	Algoritem: Quick Hull
// ----------------------------------------

// Razdalja med tocko in daljico
double DistanceToLine(const wxPoint& p, const wxPoint& a, const wxPoint& b) {
	return std::abs(CrossProduct(a, b, p)) / std::sqrt(std::pow(b.x - a.x, 2) + std::pow(b.y - a.y, 2));
}

// ploscina trikotnika
double TriangleArea(const wxPoint& a, const wxPoint& b, const wxPoint& c) {
	return std::abs(CrossProduct(a, b, c)) / 2.0;
}

// Kot med dvema tockama 
double AngleWithRespectTo(const wxPoint& origin, const wxPoint& point) {
	return std::atan2(point.y - origin.y, point.x - origin.x);
}

// Najbolj oddaljena tocka
wxPoint FindFarthestPoint(const std::vector<wxPoint>& points, const wxPoint& a, const wxPoint& b) {
	wxPoint farthestPoint;
	double maxArea = 0.0;
	double maxAngle = 0.0;

	for (const wxPoint& p : points) {
		double area = TriangleArea(a, b, p);
		if (area > maxArea) {
			maxArea = area;
			farthestPoint = p;
			maxAngle = AngleWithRespectTo(a, p);
		}
		else if (area == maxArea) {
			double angle = AngleWithRespectTo(a, p);
			if (angle > maxAngle) {
				maxAngle = angle;
				farthestPoint = p;
			}
		}
	}

	return farthestPoint;
}

// Delilna funkcija, ki razdeli tocke v dva vektorja odvisno od pozicije glede na delilno daljico
void PartitionPoints(const std::vector<wxPoint>& points, const wxPoint& a, const wxPoint& b, std::vector<wxPoint>& left, std::vector<wxPoint>& right) {
	for (const wxPoint& p : points) {
		double cross = CrossProduct(a, b, p);
		if (cross > 0) {
			left.push_back(p);
		}
		else if (cross < 0) {
			right.push_back(p);
		}
	}
}

// Odstrani duplikate iz Hull-a
std::vector<wxPoint> RemoveDuplicates(const std::vector<wxPoint>& points) {
	std::unordered_set<std::string> seen;
	std::vector<wxPoint> uniquePoints;

	for (const auto& pt : points) {
		std::string key = std::to_string(pt.x) + "," + std::to_string(pt.y);
		if (seen.find(key) == seen.end()) {
			seen.insert(key);
			uniquePoints.push_back(pt);
		}
	}

	return uniquePoints;
}

// Uredi tocke okrog minimalne
std::vector<wxPoint> SortPointsCounterClockwise(std::vector<wxPoint>& points) {
	if (points.size() < 3) return points; 

	// najdemo min tocko
	wxPoint lowest = *std::min_element(points.begin(), points.end(), [](const wxPoint& a, const wxPoint& b) {
		return a.y < b.y || (a.y == b.y && a.x < b.x);
		});

	std::sort(points.begin(), points.end(), [&](const wxPoint& a, const wxPoint& b) {
		return AngleWithRespectTo(lowest, a) < AngleWithRespectTo(lowest, b);
		});

	return points;
}

// Rekurzivna pomozna funkcija za QuickHull
void QuickHullRecursive(const std::vector<wxPoint>& points, const wxPoint& a, const wxPoint& b, std::vector<wxPoint>& hull) {
	if (points.empty()) return;

	wxPoint farthestPoint = FindFarthestPoint(points, a, b);
	if (std::find(hull.begin(), hull.end(), farthestPoint) == hull.end()) {
		hull.push_back(farthestPoint);
	}

	std::vector<wxPoint> left1, right1, left2, right2;
	PartitionPoints(points, a, farthestPoint, left1, right1);
	PartitionPoints(points, farthestPoint, b, left2, right2);

	QuickHullRecursive(left1, a, farthestPoint, hull);
	QuickHullRecursive(left2, farthestPoint, b, hull);
}

// Glavna QuickHull funckija
std::vector<wxPoint> MainFrame::QuickHull(const std::vector<wxPoint>& points) {
	if (points.size() < 3) return {}; 

	// Najdemo tocki z min in max x- vrednostjo
	auto minMaxX = std::minmax_element(points.begin(), points.end(), [](const wxPoint& p1, const wxPoint& p2) {
		return p1.x < p2.x;
		});

	wxPoint p1 = *minMaxX.first;
	wxPoint p2 = *minMaxX.second;

	std::vector<wxPoint> hull;
	hull.push_back(p1);
	hull.push_back(p2);

	std::vector<wxPoint> left, right;
	PartitionPoints(points, p1, p2, left, right);

	QuickHullRecursive(left, p1, p2, hull);
	QuickHullRecursive(right, p2, p1, hull);

	// Uredimo Hull vektor
	hull = RemoveDuplicates(hull);
	hull = SortPointsCounterClockwise(hull);

	return hull;
}

