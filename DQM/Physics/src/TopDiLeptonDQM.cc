/*
 *  $Date: 2009/11/25 14:47:42 $
 *  $Revision: 1.7 $
 *  \author M. Marienfeld - DESY Hamburg
 */

#include "TLorentzVector.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DQM/Physics/src/TopDiLeptonDQM.h"

using namespace std;
using namespace edm;

TopDiLeptonDQM::TopDiLeptonDQM( const edm::ParameterSet& ps ) {

  parameters_ = ps;
  initialize();

  moduleName_      = ps.getUntrackedParameter<string>("moduleName");
  triggerResults_  = ps.getParameter<InputTag>("TriggerResults");
  hltPaths_        = ps.getParameter<vector<string> >("hltPaths");
  hltPaths_sig_    = parameters_.getParameter<vector<string> >("hltPaths_sig");
  hltPaths_trig_   = parameters_.getParameter<vector<string> >("hltPaths_trig");

  muons_           = ps.getParameter<edm::InputTag>("muonCollection");
  muon_pT_cut_     = ps.getParameter<double>("muon_pT_cut");
  muon_eta_cut_    = ps.getParameter<double>("muon_eta_cut");
  muon_iso_cut_    = ps.getParameter<double>("muon_iso_cut");

  elecs_           = ps.getParameter<edm::InputTag>("elecCollection");
  elec_pT_cut_     = ps.getParameter<double>("elec_pT_cut");
  elec_eta_cut_    = ps.getParameter<double>("elec_eta_cut");
  elec_iso_cut_    = ps.getParameter<double>("elec_iso_cut");

  MassWindow_up_   = ps.getParameter<double>("MassWindow_up");
  MassWindow_down_ = ps.getParameter<double>("MassWindow_down");

  for(int i=0; i<100; ++i) {
    N_sig[i]  = 0;
    N_trig[i] = 0;
    Eff[i]    = 0.;
  }

  N_mumu = 0;
  N_muel = 0;
  N_elel = 0;

}


TopDiLeptonDQM::~TopDiLeptonDQM() {

}


void TopDiLeptonDQM::initialize() {

}


void TopDiLeptonDQM::beginJob() {

  dbe_ = Service<DQMStore>().operator->();

  dbe_->setCurrentFolder(moduleName_);

  Events_     = dbe_->book1D("00_Events",     "Isolated dilepton events",         5,  0.,  5.);
  Trigs_      = dbe_->book1D("01_Trigs",      "Fired muon/electron triggers",    10,  0., 10.);
  TriggerEff_ = dbe_->book1D("02_TriggerEff", "HL Trigger Efficiencies",          5,  0.,  5.);
  TriggerEff_->setTitle("HL Trigger Efficiencies #epsilon_{signal} = #frac{[signal] && [control]}{[control]}");

  Nmuons_        = dbe_->book1D("03_Nmuons",     "Number of muons",               20,   0.,  10.);
  Nmuons_iso_    = dbe_->book1D("04_Nmuons_iso", "Number of isolated muons",      20,   0.,  10.);
  Nmuons_charge_ = dbe_->book1D("Nmuons_charge", "Number of muons * moun charge", 19, -10.,  10.);
  pT_muons_      = dbe_->book1D("pT_muons",      "P_T of muons",                  40,   0., 200.);
  eta_muons_     = dbe_->book1D("eta_muons",     "Eta of muons",                  50,  -5.,   5.);
  phi_muons_     = dbe_->book1D("phi_muons",     "Phi of muons",                  40,  -4.,   4.);

  Nelecs_        = dbe_->book1D("05_Nelecs",     "Number of electrons",           20,   0.,  10.);
  Nelecs_iso_    = dbe_->book1D("06_Nelecs_iso", "Number of isolated electrons",  20,   0.,  10.);
  Nelecs_charge_ = dbe_->book1D("Nelecs_charge", "Number of elecs * elec charge", 19, -10.,  10.);
  pT_elecs_      = dbe_->book1D("pT_elecs",      "P_T of electrons",              40,   0., 200.);
  eta_elecs_     = dbe_->book1D("eta_elecs",     "Eta of electrons",              50,  -5.,   5.);
  phi_elecs_     = dbe_->book1D("phi_elecs",     "Phi of electrons",              40,  -4.,   4.);

  MuIso_emEt03_       = dbe_->book1D("MuIso_emEt03",          "Muon emEt03",       20, 0., 20.);
  MuIso_hadEt03_      = dbe_->book1D("MuIso_hadEt03",         "Muon hadEt03",      20, 0., 20.);
  MuIso_hoEt03_       = dbe_->book1D("MuIso_hoEt03",          "Muon hoEt03",       20, 0., 20.);
  MuIso_nJets03_      = dbe_->book1D("MuIso_nJets03",         "Muon nJets03",      10, 0., 10.);
  MuIso_nTracks03_    = dbe_->book1D("MuIso_nTracks03",       "Muon nTracks03",    20, 0., 20.);
  MuIso_sumPt03_      = dbe_->book1D("MuIso_sumPt03",         "Muon sumPt03",      20, 0., 40.);
  MuIso_CombRelIso03_ = dbe_->book1D("07_MuIso_CombRelIso03", "Muon CombRelIso03", 20, 0.,  1.);

  ElecIso_cal_        = dbe_->book1D("ElecIso_cal",           "Electron Iso_cal",    21, -1., 20.);
  ElecIso_trk_        = dbe_->book1D("ElecIso_trk",           "Electron Iso_trk",    21, -2., 40.);
  ElecIso_CombRelIso_ = dbe_->book1D("08_ElecIso_CombRelIso", "Electron CombRelIso", 20,  0.,  1.);

  const int nbins = 50;

  double logmin = 0.;
  double logmax = 3.;  // 10^(3.)=1000

  float bins[nbins+1];

  for (int i = 0; i <= nbins; i++) {

    double log = logmin + (logmax-logmin)*i/nbins;
    bins[i] = std::pow(10.0, log);

  }

  dimassRC_       = dbe_->book1D("09_dimassRC",      "Dimuon mass RC",        50, 0., 200.);
  dimassWC_       = dbe_->book1D("11_dimassWC",      "Dimuon mass WC",        50, 0., 200.);
  dimassRC_LOGX_  = dbe_->book1D("10_dimassRC_LOGX", "Dimuon mass RC LOG", nbins, &bins[0]);
  dimassWC_LOGX_  = dbe_->book1D("12_dimassWC_LOGX", "Dimuon mass WC LOG", nbins, &bins[0]);
  dimassRC_LOG10_ = dbe_->book1D("dimassRC_LOG10",   "Dimuon mass RC LOG",    50, 0.,  2.5);
  dimassWC_LOG10_ = dbe_->book1D("dimassWC_LOG10",   "Dimuon mass WC LOG",    50, 0.,  2.5);

  D_eta_muons_  = dbe_->book1D("13_D_eta_muons", "#Delta eta_muons", 20, -5., 5.);
  D_phi_muons_  = dbe_->book1D("14_D_phi_muons", "#Delta phi_muons", 20, -5., 5.);
  D_eta_elecs_  = dbe_->book1D("D_eta_elecs",    "#Delta eta_elecs", 20, -5., 5.);
  D_phi_elecs_  = dbe_->book1D("D_phi_elecs",    "#Delta phi_elecs", 20, -5., 5.);
  D_eta_lepts_  = dbe_->book1D("D_eta_lepts",    "#Delta eta_lepts", 20, -5., 5.);
  D_phi_lepts_  = dbe_->book1D("D_phi_lepts",    "#Delta phi_lepts", 20, -5., 5.);
}


void TopDiLeptonDQM::beginRun(const edm::Run& r, const EventSetup& context) {

}


void TopDiLeptonDQM::analyze(const edm::Event& evt, const edm::EventSetup& context) {

  // ------------------------
  //  Global Event Variables
  // ------------------------

  const int N_TriggerPaths = hltPaths_.size();
  const int N_SignalPaths  = hltPaths_sig_.size();
  const int N_ControlPaths = hltPaths_trig_.size();

  bool Fired_Signal_Trigger[100]  = {false};
  bool Fired_Control_Trigger[100] = {false};

  int N_leptons = 0;

  int N_iso_mu  = 0;
  int N_iso_el  = 0;
  int N_iso_lep = 0;

  double DilepMass = 0.;

  // -------------------------
  //  Analyze Trigger Results
  // -------------------------

  edm::Handle<TriggerResults> trigResults;
  evt.getByLabel(triggerResults_, trigResults);

  if( trigResults.failedToGet() ) {

    //    cout << endl << "-----------------------------" << endl;
    //    cout << "--- NO TRIGGER RESULTS !! ---" << endl;
    //    cout << "-----------------------------" << endl << endl;

  }

  if( !trigResults.failedToGet() ) {

    int N_Triggers = trigResults->size();

    TriggerNames trigName;
    trigName.init(*trigResults);

    for( int i_Trig = 0; i_Trig < N_Triggers; ++i_Trig ) {

      if (trigResults.product()->accept(i_Trig)) {

	// Check for all trigger paths

	for( int i = 0; i < N_TriggerPaths; i++ ) {

	  if ( trigName.triggerName(i_Trig)== hltPaths_[i] ) {

	    Trigs_->Fill(i);
	    Trigs_->setBinLabel( i+1, hltPaths_[i], 1);

	    //	    cout << "Trigger: " << hltPaths_[i] << " FIRED!!! " << endl;

	  }

	}

	// Check for signal & control trigger paths

	for( int j = 0; j < N_SignalPaths; ++j ) {

	  if( trigName.triggerName(i_Trig) == hltPaths_sig_[j]  )  Fired_Signal_Trigger[j]  = true;

	}

	for( int k = 0; k < N_ControlPaths; ++k ) {

	  if( trigName.triggerName(i_Trig) == hltPaths_trig_[k] )  Fired_Control_Trigger[k] = true;

	}

      }

    }

  }

  // ------------------------
  //  Analyze Muon Isolation
  // ------------------------

  edm::Handle<reco::MuonCollection> muons;
  evt.getByLabel(muons_, muons);

  reco::MuonCollection::const_iterator muon;

  if( muons.failedToGet() ) {

    //    cout << endl << "------------------------" << endl;
    //    cout << "--- NO RECO MUONS !! ---" << endl;
    //    cout << "------------------------" << endl << endl;

  }

  if( !muons.failedToGet() ) {

    Nmuons_->Fill( muons->size() );

    //    cout << "---------------" << endl;
    //    cout << "Nmuons    : " << muons->size() << endl;
    //    cout << "---------------" << endl << endl;

    N_leptons = N_leptons + muons->size();

    for(muon = muons->begin(); muon!= muons->end(); ++muon) {

      float N_muons = muons->size();
      float Q_muon  = muon->charge();

      Nmuons_charge_->Fill(N_muons*Q_muon);

      reco::MuonIsolation muIso03 = muon->isolationR03();

      double muonCombRelIso = 1.;

      if ( muon->pt() != 0. )
	muonCombRelIso = ( muIso03.emEt + muIso03.hadEt + muIso03.hoEt + muIso03.sumPt ) / muon->pt();

      //      cout << "MuonCombRelIso: " << muonCombRelIso << endl;

      MuIso_CombRelIso03_->Fill( muonCombRelIso );

      MuIso_emEt03_->Fill(    muIso03.emEt );
      MuIso_hadEt03_->Fill(   muIso03.hadEt );
      MuIso_hoEt03_->Fill(    muIso03.hoEt );
      MuIso_nJets03_->Fill(   muIso03.nJets );
      MuIso_nTracks03_->Fill( muIso03.nTracks );
      MuIso_sumPt03_->Fill(   muIso03.sumPt );

      if( muonCombRelIso < muon_iso_cut_ )  ++N_iso_mu;

    }

    Nmuons_iso_->Fill(N_iso_mu);

    //    cout << "Nmuons_iso: " << N_iso_mu << endl;

  }

  // ----------------------------
  //  Analyze Electron Isolation
  // ----------------------------

  edm::Handle<reco::GsfElectronCollection> elecs;
  evt.getByLabel(elecs_, elecs);

  reco::GsfElectronCollection::const_iterator elec;

  if( elecs.failedToGet() ) {

    //    cout << endl << "----------------------------" << endl;
    //    cout << "--- NO RECO ELECTRONS !! ---" << endl;
    //    cout << "----------------------------" << endl << endl;

  }

  if( !elecs.failedToGet() ) {

    Nelecs_->Fill( elecs->size() );

    //    cout << "---------------" << endl;
    //    cout << "Nelecs    : " << elecs->size() << endl;
    //    cout << "---------------" << endl << endl;

    N_leptons = N_leptons + elecs->size();

    for(elec = elecs->begin(); elec!= elecs->end(); ++elec) {

      float N_elecs = elecs->size();
      float Q_elec  = elec->charge();

      Nelecs_charge_->Fill(N_elecs*Q_elec);

      reco::GsfElectron::IsolationVariables elecIso = elec->dr03IsolationVariables();

      double elecCombRelIso = 1.;

      if ( elec->et() != 0. )
	elecCombRelIso = ( elecIso.ecalRecHitSumEt + elecIso.hcalDepth1TowerSumEt + elecIso.tkSumPt ) / elec->et();

      //      cout << "ElecCombRelIso: " << elecCombRelIso << endl;

      ElecIso_CombRelIso_->Fill( elecCombRelIso );

      ElecIso_cal_->Fill( elecIso.ecalRecHitSumEt );
      ElecIso_trk_->Fill( elecIso.tkSumPt );

      if( elecCombRelIso < elec_iso_cut_ )  ++N_iso_el;

    }

    Nelecs_iso_->Fill(N_iso_el);

    //    cout << "Nelecs_iso: " << N_iso_el << endl;

  }

  N_iso_lep = N_iso_el + N_iso_mu;

  //  cout << "---------------" << endl;
  //  cout << "Nleptons  : "     << N_leptons << endl;
  //  cout << "Nlep_iso  : "     << N_iso_lep << endl;
  //  cout << "---------------" << endl << endl;


  // --------------------
  //  TWO Isolated MUONS
  // --------------------

  //  if( N_iso_mu > 1 && Fired_Control_Trigger[0] ) {
  if( N_iso_mu > 1 ) {

    ++N_mumu;

    Events_->Fill(1.);
    Events_->setBinLabel( 2, "#mu #mu", 1);

    reco::MuonCollection::const_reference mu1 = muons->at(0);
    reco::MuonCollection::const_reference mu2 = muons->at(1);

    DilepMass = sqrt( (mu1.energy()+mu2.energy())*(mu1.energy()+mu2.energy())
		      - (mu1.px()+mu2.px())*(mu1.px()+mu2.px())
		      - (mu1.py()+mu2.py())*(mu1.py()+mu2.py())
		      - (mu1.pz()+mu2.pz())*(mu1.pz()+mu2.pz())
		      );

    // Opposite muon charges -> Right Charge (RC)

    if( mu1.charge()*mu2.charge() < 0. ) {

      dimassRC_LOG10_->Fill( log10(DilepMass) );
      dimassRC_->Fill(      DilepMass );
      dimassRC_LOGX_->Fill( DilepMass );

      if( DilepMass > MassWindow_down_ && DilepMass < MassWindow_up_ ) {

	//	cout << "DilepMass: " << DilepMass << endl;

	for(muon = muons->begin(); muon!= muons->end(); ++muon) {

	  if(     muon->pt()   < muon_pT_cut_     )  continue;
	  if( abs(muon->eta()) > muon_eta_cut_    )  continue;

	  pT_muons_->Fill(  muon->pt() );
	  eta_muons_->Fill( muon->eta() );
	  phi_muons_->Fill( muon->phi() );

	}

	D_eta_muons_->Fill(mu1.eta()-mu2.eta());
	D_phi_muons_->Fill(mu1.phi()-mu2.phi());

	// Determinating trigger efficiencies

	//	cout << "-----------------------------"   << endl;

	for( int k = 0; k < N_SignalPaths; ++k ) {

	  if( Fired_Signal_Trigger[k] && Fired_Control_Trigger[k] )  ++N_sig[k];

	  if( Fired_Control_Trigger[k] )  ++N_trig[k];

	  if( N_trig[k] != 0 )  Eff[k] = N_sig[k]/static_cast<float>(N_trig[k]);

	  //	  cout << "Signal Trigger  : " << hltPaths_sig_[k]  << "\t: " << N_sig[k]  << endl;
	  //	  cout << "Control Trigger : " << hltPaths_trig_[k] << "\t: " << N_trig[k] << endl;
	  //	  cout << "Trigger Eff.cy  : " << Eff[k]  << endl;
	  //	  cout << "-----------------------------" << endl;

	  TriggerEff_->setBinContent( k+1, Eff[k] );
	  TriggerEff_->setBinLabel( k+1, "#frac{["+hltPaths_sig_[k]+"]}{vs. ["+hltPaths_trig_[k]+"]}", 1);

	}

      }

    }

    // Same muon charges -> Wrong Charge (WC)

    if( mu1.charge()*mu2.charge() > 0. ) {

      dimassWC_LOG10_->Fill( log10(DilepMass) );
      dimassWC_->Fill(      DilepMass );
      dimassWC_LOGX_->Fill( DilepMass );

    }

  }

  // -----------------------------
  //  TWO Isolated LEPTONS (mu/e)
  // -----------------------------

  //  if( N_iso_el > 0 && N_iso_mu > 0 && Fired_Control_Trigger[0] ) {
  if( N_iso_el > 0 && N_iso_mu > 0 ) {

    //    cout << "+++ I am a mu/e EVENT !!! +++" << endl;

    ++N_muel;

    Events_->Fill(2.);
    Events_->setBinLabel( 3, "#mu e", 1);

    reco::MuonCollection::const_reference        mu1 = muons->at(0);
    reco::GsfElectronCollection::const_reference el1 = elecs->at(0);

    DilepMass = sqrt( (mu1.energy()+el1.energy())*(mu1.energy()+el1.energy())
		      - (mu1.px()+el1.px())*(mu1.px()+el1.px())
		      - (mu1.py()+el1.py())*(mu1.py()+el1.py())
		      - (mu1.pz()+el1.pz())*(mu1.pz()+el1.pz())
		      );

    // Opposite lepton charges -> Right Charge (RC)

    if( mu1.charge()*el1.charge() < 0. ) {

      dimassRC_LOG10_->Fill( log10(DilepMass) );
      dimassRC_->Fill(      DilepMass );
      dimassRC_LOGX_->Fill( DilepMass );

      if( DilepMass > MassWindow_down_ && DilepMass < MassWindow_up_ ) {

	//	cout << "DilepMass: " << DilepMass << endl;

	for(muon = muons->begin(); muon!= muons->end(); ++muon) {

	  if(     muon->pt()   < muon_pT_cut_     )  continue;
	  if( abs(muon->eta()) > muon_eta_cut_    )  continue;

	  pT_muons_->Fill(  muon->pt() );
	  eta_muons_->Fill( muon->eta() );
	  phi_muons_->Fill( muon->phi() );

	}

	for(elec = elecs->begin(); elec!= elecs->end(); ++elec) {

	  if(     elec->pt()   < elec_pT_cut_     )  continue;
	  if( abs(elec->eta()) > elec_eta_cut_    )  continue;

	  pT_elecs_->Fill(  elec->pt() );
	  eta_elecs_->Fill( elec->eta() );
	  phi_elecs_->Fill( elec->phi() );

	}

	D_eta_lepts_->Fill(mu1.eta()-el1.eta());
	D_phi_lepts_->Fill(mu1.phi()-el1.phi());

	// Determinating trigger efficiencies

	//	cout << "-----------------------------"   << endl;

	for( int k = 0; k < N_SignalPaths; ++k ) {

	  if( Fired_Signal_Trigger[k] && Fired_Control_Trigger[k] )  ++N_sig[k];

	  if( Fired_Control_Trigger[k] )  ++N_trig[k];

	  if( N_trig[k] != 0 )  Eff[k] = N_sig[k]/static_cast<float>(N_trig[k]);

	  //	  cout << "Signal Trigger  : " << hltPaths_sig_[k]  << "\t: " << N_sig[k]  << endl;
	  //	  cout << "Control Trigger : " << hltPaths_trig_[k] << "\t: " << N_trig[k] << endl;
	  //	  cout << "Trigger Eff.cy  : " << Eff[k]  << endl;
	  //	  cout << "-----------------------------" << endl;

	  TriggerEff_->setBinContent( k+1, Eff[k] );
	  TriggerEff_->setBinLabel( k+1, "#frac{["+hltPaths_sig_[k]+"]}{vs. ["+hltPaths_trig_[k]+"]}", 1);

	}

      }

    }

    // Same muon charges -> Wrong Charge (WC)

    if( mu1.charge()*el1.charge() > 0. ) {

      dimassWC_LOG10_->Fill( log10(DilepMass) );
      dimassWC_->Fill(      DilepMass );
      dimassWC_LOGX_->Fill( DilepMass );

    }

  }

  // ------------------------
  //  TWO Isolated ELECTRONS
  // ------------------------

  //  if( N_iso_el > 1 && Fired_Control_Trigger[0] ) {
  if( N_iso_el > 1 ) {

    ++N_elel;

    Events_->Fill(3.);
    Events_->setBinLabel( 4, "e e", 1);

    reco::GsfElectronCollection::const_reference el1 = elecs->at(0);
    reco::GsfElectronCollection::const_reference el2 = elecs->at(1);

    DilepMass = sqrt( (el1.energy()+el2.energy())*(el1.energy()+el2.energy())
		      - (el1.px()+el2.px())*(el1.px()+el2.px())
		      - (el1.py()+el2.py())*(el1.py()+el2.py())
		      - (el1.pz()+el2.pz())*(el1.pz()+el2.pz())
		      );

    // Opposite lepton charges -> Right Charge (RC)

    if( el1.charge()*el2.charge() < 0. ) {

      dimassRC_LOG10_->Fill( log10(DilepMass) );
      dimassRC_->Fill(      DilepMass );
      dimassRC_LOGX_->Fill( DilepMass );

      if( DilepMass > MassWindow_down_ && DilepMass < MassWindow_up_ ) {

	//	cout << "DilepMass: " << DilepMass << endl;

	for(elec = elecs->begin(); elec!= elecs->end(); ++elec) {

	  if(     elec->pt()   < elec_pT_cut_     )  continue;
	  if( abs(elec->eta()) > elec_eta_cut_    )  continue;

	  pT_elecs_->Fill(  elec->pt() );
	  eta_elecs_->Fill( elec->eta() );
	  phi_elecs_->Fill( elec->phi() );

	}

	D_eta_elecs_->Fill(el1.eta()-el2.eta());
	D_phi_elecs_->Fill(el1.phi()-el2.phi());

	// Determinating trigger efficiencies

	//	cout << "-----------------------------"   << endl;

	for( int k = 0; k < N_SignalPaths; ++k ) {

	  if( Fired_Signal_Trigger[k] && Fired_Control_Trigger[k] )  ++N_sig[k];

	  if( Fired_Control_Trigger[k] )  ++N_trig[k];

	  if( N_trig[k] != 0 )  Eff[k] = N_sig[k]/static_cast<float>(N_trig[k]);

	  //	  cout << "Signal Trigger  : " << hltPaths_sig_[k]  << "\t: " << N_sig[k]  << endl;
	  //	  cout << "Control Trigger : " << hltPaths_trig_[k] << "\t: " << N_trig[k] << endl;
	  //	  cout << "Trigger Eff.cy  : " << Eff[k]  << endl;
	  //	  cout << "-----------------------------" << endl;

	  TriggerEff_->setBinContent( k+1, Eff[k] );
	  TriggerEff_->setBinLabel( k+1, "#frac{["+hltPaths_sig_[k]+"]}{vs. ["+hltPaths_trig_[k]+"]}", 1);

	}

      }

    }

    // Same muon charges -> Wrong Charge (WC)

    if( el1.charge()*el2.charge() > 0. ) {

      dimassWC_LOG10_->Fill( log10(DilepMass) );
      dimassWC_->Fill(      DilepMass );
      dimassWC_LOGX_->Fill( DilepMass );

    }

  }

}


void TopDiLeptonDQM::endRun(const Run& r, const EventSetup& context) {

}

void TopDiLeptonDQM::endJob() {

}
