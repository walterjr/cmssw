# 
#
# Read from xml and insert into database using
# PopCon 
#
# This is a template, generate real test using
#
# sed 's/EcalIntercalibErrors/your-record/g' testTemplate.py > testyourrecord.py
#
# Stefano Argiro', $Id: testEcalIntercalibErrors.py,v 1.1 2008/11/14 15:46:03 argiro Exp $
#
#

import FWCore.ParameterSet.Config as cms

process = cms.Process("TEST")

process.MessageLogger=cms.Service("MessageLogger",
                              destinations=cms.untracked.vstring("cout"),
                              cout=cms.untracked.PSet(
                              treshold=cms.untracked.string("INFO")
                              )
)

process.load("CondCore.DBCommon.CondDBCommon_cfi")
process.CondDBCommon.connect = cms.string('sqlite_file:testEcalIntercalibErrors.db')
process.CondDBCommon.DBParameters.authenticationPath = cms.untracked.string('/afs/cern.ch/cms/DB/conddb')

process.source = cms.Source("EmptyIOVSource",
    timetype = cms.string('runnumber'),
    firstValue = cms.uint64(1),
    lastValue  = cms.uint64(2),
    interval = cms.uint64(1)
)

process.PoolDBOutputService = cms.Service("PoolDBOutputService",
    process.CondDBCommon,
    timetype = cms.untracked.string('runnumber'),
    toPut = cms.VPSet(cms.PSet(
        record = cms.string('EcalIntercalibErrorsRcd'),
        tag = cms.string('mytest')
         )),
    logconnect= cms.untracked.string('sqlite_file:logtestEcalIntercalibErrors.db')                                     
)

process.mytest = cms.EDAnalyzer("EcalIntercalibErrorsAnalyzer",
    record = cms.string('EcalIntercalibErrorsRcd'),
    loggingOn= cms.untracked.bool(True),
    SinceAppendMode=cms.bool(True),
    Source=cms.PSet(
    xmlFile = cms.untracked.string('/tmp/EcalIntercalibConstants.xml'),
    since = cms.untracked.int64(3)
    )                            
)

process.p = cms.Path(process.mytest)




