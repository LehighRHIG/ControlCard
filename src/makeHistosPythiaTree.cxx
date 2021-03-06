
#include "TSystem.h"
#include "TROOT.h"
#include "Tfile.h"
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TClonesArray.h"
#include "TBranch.h"
#include "TLorentzVector.h"
#include "TParameter.h"
#include <iostream>
#include <cmath>
#include <TMath.h>
#include <exception>

using namespace std;

int main(int argc, const char** argv)
{
    const char *defaults[6] = {"readPythiaTree","test.root","in"};
    if ( argc==1 ) {
        argv=defaults;
        argc=3;
    }
    TString InFileName = argv[1];
    cout<<"In file is "<<InFileName.Data()<<endl;
    
    TString OutFileName = argv[2];
    cout<<"Out file is "<<OutFileName.Data()<<endl;
    
    //TString jetspecs= "reekjed: " ;
    //jetspecs.Append()
    
    TFile *_file0 = TFile::Open(InFileName); //read tree
    TTree* tree = (TTree*)gROOT->FindObject("tree");
    
    //read collections
    TClonesArray *HardPartons = new TClonesArray("TLorentzVector");
    TBranch* bHardPartons = tree->GetBranch("HardPartons");
    bHardPartons->SetAddress(&HardPartons);
    TClonesArray *HardPartonNames = new TClonesArray("TParameter<int>");
    TBranch* bHardPartonNames = tree->GetBranch("HardPartonNames");
    bHardPartonNames->SetAddress(&HardPartonNames);
    TClonesArray *particles = new TClonesArray("TLorentzVector");
    TBranch* bparticles = tree->GetBranch("Particles");
    bparticles->SetAddress(&particles);
    TClonesArray *Jets = new TClonesArray("TLorentzVector");
    TBranch* bJets = tree->GetBranch("jets_R0100_ptmin0");
    bJets->SetAddress(&Jets);
    TClonesArray *Background = new TClonesArray("TLorentzVector");
    TBranch* bBackground = tree->GetBranch("Background");
    bBackground->SetAddress(&Background);
    
    //First create the histograms that will be filled with >> command
    TString histname("hHardPartonspT");
    TH1F* hHardPartonspT = new TH1F(histname,histname,16,3,7);
    histname.Prepend("sqrt(HardPartons.fP.fX*HardPartons.fP.fX+HardPartons.fP.fY*HardPartons.fP.fY)>>");
    tree->Draw(histname);
    
    histname = "hHardPartonsName";
    TH1F* hHardPartonsName = new TH1F(histname,histname,40,-9.5,30.5);
    histname.Prepend("HardPartonNames.fVal>>");
    tree->Draw(histname);
    
    //Next book the histograms that will be filled event by event
    histname = "hParticlepT";
    TH1F* hParticlepT = new TH1F(histname,histname,20,-5,15);
    histname = "hParticleEtaPhi";
    TH2F* hParticleEtaPhi = new TH2F(histname,histname,100,-1,1,100,-6.2,6.2);
    histname= "hParticlesPhiCount";
    TH1F* hParticlesPhiCount= new TH1F(histname,histname,160,-4,4);
    histname="hParticlesEtaCount";
    TH1F* hParticlesEtaCount= new TH1F(histname,histname,80,-2,2);
    histname="hParticlesEtaPhipT";
    TH2F* hParticlesEtaPhipT= new TH2F(histname,histname,50,-1,1,50,-6.2,6.2);

    //Jet histograms
    histname="JetpT";
    TH1F* JetpT = new TH1F(histname,histname,50,-5,45);
    histname = "JetEtaPhi";
    TH2F* JetEtaPhi = new TH2F(histname,histname,100,-1,1,100,-6.3,6.3);
    histname= "JetPhiCount";
    TH1F* JetPhiCount= new TH1F(histname,histname,160,-4,4);
    histname="JetEtaCount";
    TH1F* JetEtaCount= new TH1F(histname,histname,80,-2,2);
    histname="JetEtaPhipT";
    TH2F* JetEtaPhipT= new TH2F(histname,histname,50,-1,1,50,-6.2,6.2);
    
    //Background Histograms
    TH1* hpT = new TH1F("hpT","",100,0,20);
    TH1* hPhi = new TH1F("hPhi","",20,0,2*TMath::Pi());
    TH1* hEta = new TH1F("hEta","",20,-1.5,1.5);
    SetHist(hpT,"p_{T,track} (GeV/c)","#frac{d^{2}N}{2#pidp_{T}d#eta}",1);
    hpT->GetYaxis()->SetTitleOffset(1.8);
    SetHist(hPhi,"#phi","#frac{dN}{d#phi}");
    SetHist(hEta,"#eta","#frac{dN}{d#eta}");
    
    Int_t entries = tree->GetEntries();
    Double_t trigmin = 12.5;
    Double_t trigmax = 20.0;
    Double_t assocmin=4.0;
    Double_t assocmax=8.0;
    Double_t doublepi= 2*TMath::Pi();
    histname="deltphieta";
    Int_t binsize= 50;
    Double_t count=0.0;
    TH2F *deltphieta= new TH2F(histname,histname,binsize,-1,doublepi-1,binsize,-2,2);
    deltphieta->SetXTitle("dphi");
    deltphieta->SetYTitle("deta");
    deltphieta->SetZTitle("normalized counts");
    Int_t tej=0;
    for (Int_t iev = 0;iev<entries;iev++){
        //Looping over number of events
        if (iev % 1000 == 0)
            cout<<"Event "<<iev<<endl;
        
        //Get all the particles in the event
        tree->GetEntry(iev);
        bHardPartons->GetEntry(iev);
        bHardPartonNames->GetEntry(iev);
        bparticles->GetEntry(iev);
        bJets->GetEntry(iev);
        bBackground->GetEntry(iev);
        //loop over final state particles
        Int_t nparticles = particles->GetEntriesFast();
        Int_t njets=Jets->GetEntriesFast();
        Int_t nBackground->GetEntriesFast();
        tej=tej +njets;
        for(int i=0;i<njets;i++){
            TLorentzVector* jet=(TLorentzVector*)Jets->At(i);
            JetpT->Fill(jet->Pt());
            JetEtaPhi->Fill(jet->Eta(),jet->Phi());
            JetPhiCount->Fill(jet->Phi());
            JetEtaCount->Fill(jet->Eta());
            JetEtaPhipT->Fill(jet->Eta(),jet->Phi());
            Double_t jetpt= jet->Pt();
            Double_t jetphi= jet->Phi();
            Double_t jeteta= jet->Eta();
            if((jetpt<trigmax)&&(jetpt>trigmin)){
                for (int ip = 0; ip<nparticles;ip++){
                    //Find each particle
                    TLorentzVector* particle = (TLorentzVector*)particles->At(ip);
                    //Fill arrays
                    //momentum[iev][ip]=particle->Pt();
                    //angle[iev][ip]=particle->Phi();
                    Double_t ipt=particle->Pt();
                    Double_t iphi=particle->Phi();
                    Double_t ieta=particle->Eta();
                    if((ipt<assocmax)&&(ipt>assocmin)){
                        if((jetphi-iphi)>(doublepi-1))
                            deltphieta->Fill((jetphi-iphi-doublepi),jeteta-ieta);
                        else if((jetphi-iphi)<-1)
                            deltphieta->Fill((jetphi-iphi+doublepi),jeteta-ieta);
                        else{
                            deltphieta->Fill(jetphi-iphi,jeteta-ieta);
                        }
                    }
                    //Manipulate particles and fill histograms
                    /*hParticlepT->Fill(particle->Pt());
                     hParticleEtaPhi->Fill(particle->Eta(),particle->Phi());
                     if(ie>0)
                     hParticlesEtaPhipT->Fill(particle->Eta(),particle->Phi(),particle->E());
                     //  hParticlesPhiCount->Fill(particle->Phi());
                     //hParticlesEtaCount->Fill(particle->Eta());*/
                }
                count++;
            }
            
        }
        for(int i=0;i<nBackground;i++){
            TLorentzVector* backparticle=(TLorentzVector*)Background->At(i);
            hpT->Fill(backparticle->Pt());
            hEta->Fill(backparticle->Eta());
            hPhi->Fill(backparticle->Phi());
        }
    }//end of event loop
    deltphieta->Sumw2();
    deltphieta->Scale(1.0/(binsize*count*binsize));


    TFile *file1 = TFile::Open(OutFileName.Data(),"RECREATE");

    hHardPartonspT->Write();
    hHardPartonsName->Write();
    hParticlepT->Write();
    hParticleEtaPhi->Write();
    deltphieta->Write();
    JetpT->Write();
    JetEtaPhi->Write();
    JetPhiCount->Write();
    JetEtaCount->Write();
    hParticlesEtaPhipT->Write();
    JetEtaPhipT->Write();
    hPhi->Write();
    hpT->Write();
    hEta->Write();
}
