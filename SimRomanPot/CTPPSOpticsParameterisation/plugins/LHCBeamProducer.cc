// -*- C++ -*-
//
// Package:    SimRomanPot/CTPPSOpticsParameterisation
// Class:      LHCBeamProducer
// 
/**\class LHCBeamProducer LHCBeamProducer.cc SimRomanPot/CTPPSOpticsParameterisation/plugins/LHCBeamProducer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Laurent Forthomme
//         Created:  Wed, 24 May 2017 07:40:20 GMT
//
//


#include <memory>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/RandomNumberGenerator.h"

#include "DataFormats/CTPPSDetId/interface/TotemRPDetId.h"
#include "SimDataFormats/CTPPS/interface/CTPPSSimProtonTrack.h"

#include "CLHEP/Random/RandGauss.h"
#include "CLHEP/Random/RandFlat.h"

class LHCBeamProducer : public edm::stream::EDProducer<> {
  public:
    explicit LHCBeamProducer( const edm::ParameterSet& );
    ~LHCBeamProducer();

    static void fillDescriptions( edm::ConfigurationDescriptions& descriptions );

  private:
    virtual void beginStream( edm::StreamID ) override;
    virtual void produce( edm::Event&, const edm::EventSetup& ) override;
    virtual void endStream() override;

    CTPPSSimProtonTrack shoot( CLHEP::HepRandomEngine* );

    unsigned int numProtons_;
    edm::ParameterSet beamConditions_;

    bool simulateVertexX_, simulateVertexY_;

    bool simulateScatteringAngleX_, simulateScatteringAngleY_;
    double thetaPhys_;

    bool simulateBeamDivergence_;

    bool simulateXi_;
    double minXi_;
    double maxXi_;

    double vertexSize_;
    double beamDivergence_;
};

LHCBeamProducer::LHCBeamProducer( const edm::ParameterSet& iConfig ) :
  numProtons_( iConfig.getParameter<unsigned int>( "numProtons" ) ),
  beamConditions_( iConfig.getParameter<edm::ParameterSet>( "beamConditions" ) ),
  simulateVertexX_( iConfig.getParameter<bool>( "simulateVertexX" ) ),
  simulateVertexY_( iConfig.getParameter<bool>( "simulateVertexY" ) ),
  simulateScatteringAngleX_( iConfig.getParameter<bool>( "simulateScatteringAngleX" ) ),
  simulateScatteringAngleY_( iConfig.getParameter<bool>( "simulateScatteringAngleY" ) ),
  thetaPhys_( iConfig.getParameter<double>( "scatteringAngle" ) ),
  simulateBeamDivergence_( iConfig.getParameter<bool>( "simulateBeamDivergence" ) ),
  simulateXi_( iConfig.getParameter<bool>( "simulateXi" ) ),
  minXi_( iConfig.getParameter<double>( "minXi" ) ),
  maxXi_( iConfig.getParameter<double>( "maxXi" ) )
{
  produces< std::vector<CTPPSSimProtonTrack> >();

  vertexSize_ = beamConditions_.getParameter<double>( "vertexSize" );
  beamDivergence_ = beamConditions_.getParameter<double>( "beamDivergence" );
}


LHCBeamProducer::~LHCBeamProducer()
{}


// ------------ method called to produce the data  ------------
void
LHCBeamProducer::produce( edm::Event& iEvent, const edm::EventSetup& )
{
  edm::Service<edm::RandomNumberGenerator> rng;
  CLHEP::HepRandomEngine* rnd = &( rng->getEngine( iEvent.streamID() ) );

  std::unique_ptr< std::vector<CTPPSSimProtonTrack> > pOut( new std::vector<CTPPSSimProtonTrack> );

  for ( unsigned int i=0; i<numProtons_; i++ ) {
    pOut->push_back( shoot( rnd ) );
  }

  iEvent.put( std::move( pOut ) );
}

//----------------------------------------------------------------------------------------------------

CTPPSSimProtonTrack
LHCBeamProducer::shoot( CLHEP::HepRandomEngine* rnd )
{
  // generate vertex
  double vtx_x = 0., vtx_y = 0.;

  if ( simulateVertexX_ ) vtx_x += CLHEP::RandGauss::shoot( rnd ) * vertexSize_;
  if ( simulateVertexY_ ) vtx_y += CLHEP::RandGauss::shoot( rnd ) * vertexSize_;

  const Local3DPoint vtx( vtx_x, vtx_y, 0. );

  // generate scattering angles (physics)
  double th_x_phys = 0., th_y_phys = 0.;

  if ( simulateScatteringAngleX_ ) th_x_phys += CLHEP::RandGauss::shoot( rnd ) * thetaPhys_;
  if ( simulateScatteringAngleY_ ) th_y_phys += CLHEP::RandGauss::shoot( rnd ) * thetaPhys_;

  // generate beam divergence, calculate complete angle
  double th_x = th_x_phys, th_y = th_y_phys;

  if ( simulateBeamDivergence_ ) {
    th_x += CLHEP::RandGauss::shoot( rnd ) * beamDivergence_;
    th_y += CLHEP::RandGauss::shoot( rnd ) * beamDivergence_;
  }

  // generate xi
  double xi = 0.;
  if ( simulateXi_ ) xi = minXi_ + CLHEP::RandFlat::shoot( rnd ) * ( maxXi_-minXi_ );

  return CTPPSSimProtonTrack( vtx, Local3DVector( th_x, th_y, 0. ), xi );
}

// ------------ method called once each stream before processing any runs, lumis or events  ------------
void
LHCBeamProducer::beginStream( edm::StreamID )
{}

// ------------ method called once each stream after processing all runs, lumis and events  ------------
void
LHCBeamProducer::endStream()
{}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
LHCBeamProducer::fillDescriptions( edm::ConfigurationDescriptions& descriptions )
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault( desc );
}

// define this as a plug-in
DEFINE_FWK_MODULE( LHCBeamProducer );
