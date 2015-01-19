#include <iostream>
#include <QImage>


#include "TCanvas.h"
#include "TGraph2D.h"
#include "TH3I.h"
#include "TF2.h"
#include "TApplication.h"

using namespace std;
int main(int argc, char **argv) {
    QImage image (TESTS_SRC_DIR "/roi-morestars.png");
    //QImage image (TESTS_SRC_DIR "/roi-1star.png");
    if(image.isNull()) {
      cerr << "error loading image!" << endl;
      return 1;
    }
    
    TApplication *gMyRootApp = new TApplication("My ROOT Application", &argc, argv);

    //TH3I *th = new TH3I;
    //th->SetBins(image.width(), 0, image.width(), image.height(), 0, image.height(), 255, 0, 255);
    TGraph2D *graph = new TGraph2D;
    int i=0;
    for(int y=0; y<image.height(); y++) {
      for(int x=0; x<image.width(); x++) {
	auto pixel = qGray(image.pixel(x, y));
	cout << x << "x" << y << ": " << pixel << endl;
	//th->Fill(x, y, pixel);
	graph->SetPoint(i, x, y, pixel);
	i++;
      }
    }
    TF2 *f2 = new TF2("f2","xygaus(0)",0,image.width(),0,image.height());
    f2->SetParameters(1,3,1,4,1);
    graph->Fit(f2);
    f2->Draw("surf1");
    graph->Draw("p");
    //th->Fit(f2);
    //th->Draw("*");
    gMyRootApp->Run();
    return 0;
}