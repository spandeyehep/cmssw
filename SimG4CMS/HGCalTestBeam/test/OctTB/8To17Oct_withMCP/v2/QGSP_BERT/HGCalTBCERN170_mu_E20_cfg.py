import FWCore.ParameterSet.Config as cms

process = cms.Process('SIM')

# import of standard configurations
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('SimGeneral.MixingModule.mixNoPU_cfi')
process.load('SimG4CMS.HGCalTestBeam.HGCalTB181Oct8To17WithMCPXML_cfi')
process.load('Geometry.HGCalCommonData.hgcalNumberingInitialization_cfi')
process.load('Geometry.HGCalCommonData.hgcalParametersInitialization_cfi')
process.load('Configuration.StandardSequences.MagneticField_0T_cff')
process.load('Configuration.StandardSequences.Generator_cff')
process.load('IOMC.EventVertexGenerators.VtxSmearedFlat_cfi')
#process.load('IOMC.EventVertexGenerators.VtxSmearedGauss_cfi')
process.load('GeneratorInterface.Core.genFilterSummary_cff')
process.load('Configuration.StandardSequences.SimIdeal_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('SimG4CMS.HGCalTestBeam.HGCalTBCheckGunPosition_cfi')
process.load('SimG4CMS.HGCalTestBeam.HGCalTBAnalyzer_cfi')

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(10)
)

#if 'MessageLogger' in process.__dict__:
#    process.MessageLogger.categories.append('HGCSim')
#    process.MessageLogger.categories.append('HcalSim')

# Input source
process.source = cms.Source("EmptySource")

process.options = cms.untracked.PSet(
)

# Production Info
process.configurationMetadata = cms.untracked.PSet(
    annotation = cms.untracked.string('SingleMuonE200_cfi nevts:10'),
    name = cms.untracked.string('Applications'),
    version = cms.untracked.string('$Revision: 1.19 $')
)

# Output definition

process.RAWSIMoutput = cms.OutputModule("PoolOutputModule",
    SelectEvents = cms.untracked.PSet(
        SelectEvents = cms.vstring('generation_step')
    ),
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('GEN-SIM'),
        filterName = cms.untracked.string('')
    ),
    eventAutoFlushCompressedSize = cms.untracked.int32(5242880),
    fileName = cms.untracked.string('file:gensim.root'),
    outputCommands = process.RAWSIMEventContent.outputCommands,
    splitLevel = cms.untracked.int32(0)
)

# Additional output definition
process.TFileService = cms.Service("TFileService",
                                   fileName = cms.string('TBGenSim.root')
                                   )

# Other statements
process.genstepfilter.triggerConditions=cms.vstring("generation_step")
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_mc', '')

process.generator = cms.EDProducer("FlatRandomEThetaGunProducer",
#process.generator = cms.EDProducer("GaussRandomPThetaGunProducer",
    AddAntiParticle = cms.bool(False),
    PGunParameters = cms.PSet(
		MinE = cms.double(20),
		MaxE = cms.double(20),
		#MeanP    = cms.double(MEANP),
		#SigmaP   = cms.double(SIGMAP),
		
		MinTheta = cms.double(0.0),
		MaxTheta = cms.double(0.0),
		MinPhi = cms.double(-3.14159265359),
		MaxPhi = cms.double(3.14159265359),
		PartID = cms.vint32(13)
		),
				   Verbosity = cms.untracked.int32(0),
				   firstRun = cms.untracked.uint32(1),
				   psethack = cms.string('single muon E 100')
				   )

#process.VtxSmeared.MinZ = -800.0
#process.VtxSmeared.MaxZ = -800.0
#process.VtxSmeared.MinX = -7.5
#process.VtxSmeared.MaxX =  7.5
#process.VtxSmeared.MinY = -7.5
#process.VtxSmeared.MaxY =  7.5


process.VtxSmeared.MinZ = 1000.0
process.VtxSmeared.MaxZ = 1000.0
process.VtxSmeared.MinX = -5
process.VtxSmeared.MaxX =  5
process.VtxSmeared.MinY = -5
process.VtxSmeared.MaxY =  5



#process.VtxSmeared.MeanX  = .0493735
#process.VtxSmeared.SigmaX = 1.29778
#process.VtxSmeared.MeanY  = .358507
#process.VtxSmeared.SigmaY = 1.37312
##process.VtxSmeared.MeanZ  = -800.0
#process.VtxSmeared.MeanZ  = 1000.0
#process.VtxSmeared.SigmaZ = 0


process.g4SimHits.HGCSD.RejectMouseBite = True
process.g4SimHits.HGCSD.RotatedWafer    = True
process.g4SimHits.Watchers = cms.VPSet(cms.PSet(
		HGCPassive = cms.PSet(
			#LVNames = cms.vstring('HGCalEE','HGCalHE','HGCalAH', 'CMSE'),
			LVNames = cms.vstring('HGCalEE','HGCalHE', 'HGCalBeam', 'CMSE'),
			MotherName = cms.string('CMSE'),
			),
		type = cms.string('HGCPassive'),
		)
				       )
process.HGCalTBAnalyzer.DoDigis     = False
process.HGCalTBAnalyzer.DoRecHits   = False
process.HGCalTBAnalyzer.UseFH       = True
process.HGCalTBAnalyzer.UseBH       = False
process.HGCalTBAnalyzer.UseBeam     = True
process.HGCalTBAnalyzer.ZFrontEE    = 1110.0
#process.HGCalTBAnalyzer.ZFrontFH    = 1148.3
process.HGCalTBAnalyzer.ZFrontFH    = 1172.3
process.HGCalTBAnalyzer.DoPassive   = True
process.HGCalTBAnalyzer.DoPassiveEE   = True
process.HGCalTBAnalyzer.DoPassiveHE   = True
process.HGCalTBAnalyzer.DoPassiveBH   = False


# Path and EndPath definitions
process.generation_step = cms.Path(process.pgen)
process.gunfilter_step  = cms.Path(process.HGCalTBCheckGunPostion)
process.simulation_step = cms.Path(process.psim)
process.genfiltersummary_step = cms.EndPath(process.genFilterSummary)
process.analysis_step = cms.Path(process.HGCalTBAnalyzer)
process.endjob_step = cms.EndPath(process.endOfProcess)
process.RAWSIMoutput_step = cms.EndPath(process.RAWSIMoutput)

# Schedule definition
process.schedule = cms.Schedule(process.generation_step,
				process.genfiltersummary_step,
				process.simulation_step,
				#process.gunfilter_step,
				process.analysis_step,
				process.endjob_step
#				,process.RAWSIMoutput_step,
				)
# filter all path with the production filter sequence
for path in process.paths:
	getattr(process,path)._seq = process.generator * getattr(process,path)._seq 


