#pragma once
#include<wx/wx.h> // tukaj rabimo specificno frame.h
#include "PointPanel.h"



class MainFrame : public wxFrame
{
public:
	MainFrame(const wxString& title);
private:
	//Nam generira tocke 
	void OnGeneratePoints(wxCommandEvent& evt);
	void GenerateNormalPoints(int numPoints, std::vector<wxPoint>& points);
	void GenerateUniformPoints(int numPoints, std::vector<wxPoint>& points);

	//Algoritmi
	void OnCalculateHull(wxCommandEvent& evt);

	std::vector<wxPoint> JarvisMarch(const std::vector<wxPoint>& points);
	std::vector<wxPoint> generatedPoints; // array generiranih tock

	std::vector<wxPoint> GrahamScan(const std::vector<wxPoint>& points);

	std::vector<wxPoint> QuickHull(const std::vector<wxPoint>& points);


	wxRadioButton* rbJarvis;
	wxRadioButton* rbGraham;
	wxRadioButton* rbQuick;

	wxRadioButton* rbNormal;
	wxRadioButton* rbUniform;

	wxSpinCtrl* spinCtrl;

	PointPanel* pointPanel;
	wxTextCtrl* infoTextCtrl;
};

