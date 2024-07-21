#include "App.h"
#include "MainFrame.h"
#include <wx/wx.h>
// ta koda mi zgenerira GUI

wxIMPLEMENT_APP(App);

bool App::OnInit()
{
	MainFrame* mainFrame = new MainFrame("Projekt: Izbocena Lupina"); // ustvari GUI
	mainFrame->SetClientSize(800, 600);				// modificiras velikost okenca
	mainFrame->Center();							// centrira okence na sredino zaslona
	mainFrame->Show();								//prikaze GUI
	return true;
}