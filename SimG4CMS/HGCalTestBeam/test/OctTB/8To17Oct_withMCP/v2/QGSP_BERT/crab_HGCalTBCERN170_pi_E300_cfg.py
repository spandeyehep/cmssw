from WMCore.Configuration import Configuration
config = Configuration()

config.section_("General")
config.General.requestName = 'HGCalTB_sim_pion300'
config.General.workArea = 'crab_projects'

#optional
#config.General.transferOutputs
#config.General.transferLogs
#config.General.failureLimit = 

#Expert use
#config.General.instance
#config.General.activity

config.section_("JobType")
config.JobType.pluginName = 'PrivateMC'
#config.JobType.psetName = 'SinglePiE300HCAL_pythia8_cfi_GEN_SIM.py'
config.JobType.psetName = 'HGCalTBCERN170_pi_E300_cfg.py'
config.JobType.outputFiles = ['TBGenSim.root']
config.JobType.eventsPerLumi = 2000

config.section_("Data")
#config.Data.inputDataset = '/Single_Pion_gun_13TeV_pythia8/Fall14DR73-NoPU_MCRUN2_73_V9-v1/GEN-SIM-RAW-RECO'
#config.Data.primaryDataset = ''
config.Data.splitting = 'EventBased'
config.Data.unitsPerJob = 2000
NJOBS = 50
config.Data.totalUnits = config.Data.unitsPerJob * NJOBS
config.Data.publication = False
#config.Data.publishDBS = '' default for the moment
#config.Data.outLFN = '/home/spandey/t3store/PF_PGun'
config.Data.outLFNDirBase = '/store/user/spandey/HGCAL/simulation_v3/QGSP_BERT/HGCalTB_sim_pion300/'

config.section_("Site")
config.Site.storageSite = 'T2_IN_TIFR'
#config.Site.blacklist = ['T3_US_UCR', 'T3_US_UMiss']
#config.Site.whitelist = ['T2_CH_CERN','T2_KR_KNU']

#config.section_("User")
#config.section_("Debug")
