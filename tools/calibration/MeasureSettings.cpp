// Calibration measurement tool (developer only; built with -DECHOPSYCHFX_BUILD_TESTS=ON).
//
// Renders audio through the real plugin and prints one CSV row per settings sample: the predicted
// SoundCharacter fields followed by what was actually measured (width change in dB, channel
// correlation, brightness shift, level change, reverb/echo tail seconds, motion). The models in
// src/PerceptionProfile.cpp were fitted to rows produced by this tool - see README.md.
//
//   MeasureSettings <seed> <count>      random settings (half of them perturbed factory presets)
//   MeasureSettings 0 0 presets         every factory preset
#include "PluginProcessor.h"
#include "PerceptionPresetManager.h"
#include "PerceptionProfile.h"
#include <cstdio>
#include <cmath>
#include <random>
#include <vector>
#include <string>
using Sig = std::vector<float>;
static const double SR = 44100.0; static const int BLOCK = 512;
struct Out { Sig l, r; };
static Out run(AudioPluginAudioProcessor& p, const Sig& inL, const Sig& inR){
    Out o; juce::MidiBuffer midi; juce::AudioBuffer<float> b(2,BLOCK); size_t n=inL.size();
    for(size_t pos=0; pos<n; pos+=BLOCK){ int len=(int)std::min<size_t>(BLOCK,n-pos); b.clear();
        for(int i=0;i<len;++i){ b.setSample(0,i,inL[pos+i]); b.setSample(1,i,inR[pos+i]); }
        p.processBlock(b,midi); for(int i=0;i<len;++i){ o.l.push_back(b.getSample(0,i)); o.r.push_back(b.getSample(1,i)); } }
    return o; }
static double rmsr(const Sig& s,size_t a,size_t b){ double e=0; for(size_t i=a;i<b;++i) e+=s[i]*s[i]; return std::sqrt(e/std::max<size_t>(1,b-a)); }
static double db(double x){ return 20*std::log10(x+1e-12); }
static void widthStats(const Out& o,size_t a,size_t b,double& smDb,double& corr){
    double ss=0,mm=0,ll=0,rr=0,lr=0; for(size_t i=a;i<b;++i){ double m=(o.l[i]+o.r[i])*0.5,s=(o.l[i]-o.r[i])*0.5; ss+=s*s; mm+=m*m; ll+=o.l[i]*o.l[i]; rr+=o.r[i]*o.r[i]; lr+=o.l[i]*o.r[i]; }
    smDb=10*std::log10((ss+1e-18)/(mm+1e-18)); corr=lr/std::sqrt(ll*rr+1e-18); }
static void bands(const Out& o,size_t a,size_t b,double& lowDb,double& highDb){
    const int order=13, N=1<<order; juce::dsp::FFT fft(order); std::vector<float> buf(2*N); std::vector<double> acc(N/2,0.0); int count=0;
    for(size_t pos=a; pos+N<=b; pos+=N/2){ for(int i=0;i<N;++i){ double w=0.5-0.5*std::cos(2*M_PI*i/N); buf[i]=(float)(0.5*(o.l[pos+i]+o.r[pos+i])*w);} std::fill(buf.begin()+N,buf.end(),0.f); fft.performFrequencyOnlyForwardTransform(buf.data()); for(int k=0;k<N/2;++k) acc[k]+=buf[k]*buf[k]; ++count; }
    auto band=[&](double f0,double f1){ double e=0; for(int k=(int)(f0/SR*N);k<(int)(f1/SR*N);++k) e+=acc[k]; return 10*std::log10(e/std::max(1,count)+1e-18); };
    lowDb=band(100,500); highDb=band(4000,12000); }
static void resetTo(AudioPluginAudioProcessor& p){ p.prepareToPlay(SR,BLOCK); juce::MidiBuffer m; juce::AudioBuffer<float> b(2,BLOCK); for(int i=0;i<200;++i){ b.clear(); p.processBlock(b,m);} }

int main(int argc,char** argv){
    juce::ScopedJuceInitialiser_GUI init;
    int seed=std::atoi(argv[1]), count=std::atoi(argv[2]); bool presets = argc>3;
    AudioPluginAudioProcessor p; p.setPlayConfigDetails(2,2,SR,BLOCK); p.prepareToPlay(SR,BLOCK);
    PerceptionPresetManager mgr(p.parameters);
    std::mt19937 rng(seed); std::normal_distribution<float> g(0.f,0.12f); std::uniform_real_distribution<float> U(0,1);
    const size_t N=(size_t)(5*SR); Sig sL(N),sR(N),mono(N);
    for(size_t i=0;i<N;++i){ float a=g(rng),c=g(rng); sL[i]=a; sR[i]=0.5f*a+0.866f*c; mono[i]=a; }
    const size_t TN=(size_t)(8*SR); Sig bL(TN,0.f),bR(TN,0.f); for(size_t i=0;i<(size_t)(0.4*SR);++i){ bL[i]=g(rng); bR[i]=g(rng); }
    double inSM,inCorr,inLow,inHigh; { Out in{sL,sR}; widthStats(in,N/2,N,inSM,inCorr); bands(in,N/2,N,inLow,inHigh);} double inLevel=db(rmsr(sL,N/2,N));
    std::vector<juce::RangedAudioParameter*> params; for(auto* q: p.getParameters()) if(auto* r=dynamic_cast<juce::RangedAudioParameter*>(q)) params.push_back(r);
    auto presetList=getFactoryPresets();
    int total = presets ? (int)presetList.size() : count;
    if(seed==0 && !presets) printf("id,mono,effWidth,sideVsMidDb,tilt,spatialMix,spatialDepth,spatialRate,haasLead,phaseSpread,delayMix,delayFb,delayDepth,delayRate,delayTime,microMix,detune,diffusion,sep,lfoDepth,lfoRate,exMix,exDrive,harmDb,wet,size,damping,predelay,exHP,exBal,exBright,sfxAllpass,delayCentre,dSM,corr,dSMmono,dHF,dLevel,tail40,tailE,motion\n");
    for(int k=0;k<total;++k){
        if(presets) mgr.applyPreset(presetList[k].name);
        else { bool near = U(rng)<0.5f; if(near) mgr.applyPreset(presetList[(int)(U(rng)*presetList.size())%presetList.size()].name);
          for(auto* r: params){ float v=U(rng); const auto& id=r->paramID;
            if(near){ float base=r->getValue(); v=std::min(1.f,std::max(0.f,base+0.14f*std::normal_distribution<float>(0.f,1.f)(rng))); if(id=="mono"||id=="sync"||id=="modulationType"||id=="modulationShape"||id=="exciterSaturationType"||id=="exciterHarmonicMode"||id=="exciterAutoGain") v=base; r->setValueNotifyingHost(v); continue; }
            bool isMix = id=="modMix"||id=="mix"||id=="exciterMix"||id=="wet"||id=="sfxWetDryMix";
            if(isMix){ v = (U(rng)<0.35f) ? 0.f : 0.05f+0.95f*U(rng); }
            if(id=="mono") v = U(rng)<0.12f ? 1.f : 0.f;
            if(id=="sync") v = U(rng)<0.2f ? 1.f : 0.f;
            r->setValueNotifyingHost(v); } }
        auto getter=[&](const char* id){ return p.parameters.getRawParameterValue(id)->load(); };
        auto c=computeSoundCharacter(getter);
        resetTo(p); auto o=run(p,sL,sR); double sm,corr,lo,hi; widthStats(o,N/2,N,sm,corr); bands(o,N/2,N,lo,hi);
        double dHF=(hi-lo)-(inHigh-inLow), dLev=db(rmsr(o.l,N/2,N))-inLevel;
        std::vector<double> w; for(size_t pos=N/2; pos+4410<=N; pos+=4410){ double s,cc; widthStats(o,pos,pos+4410,s,cc); w.push_back(s);} double mean=0; for(double x:w) mean+=x; mean/=w.size(); double var=0; for(double x:w) var+=(x-mean)*(x-mean); double motion=std::sqrt(var/w.size());
        resetTo(p); auto om=run(p,mono,mono); double smM,cM; widthStats(om,N/2,N,smM,cM);
        resetTo(p); auto ot=run(p,bL,bR); double peak=db(0.5*(rmsr(ot.l,0,(size_t)(0.4*SR))+rmsr(ot.r,0,(size_t)(0.4*SR)))); double tail=0;
        for(size_t pos=(size_t)(0.4*SR); pos+2205<=TN; pos+=2205){ double lv=db(0.5*(rmsr(ot.l,pos,pos+2205)+rmsr(ot.r,pos,pos+2205))); if(lv>peak-40) tail=(pos+2205)/SR-0.4; }
        double eBurst=0,eAfter=0; for(size_t i=0;i<(size_t)(0.4*SR);++i) eBurst+=ot.l[i]*ot.l[i]+ot.r[i]*ot.r[i]; for(size_t i=(size_t)(0.4*SR);i<TN;++i) eAfter+=ot.l[i]*ot.l[i]+ot.r[i]*ot.r[i];
        double tailE=10*std::log10((eAfter+1e-18)/(eBurst+1e-18));
        printf("%d,%d,%.4f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.4f,%.3f,%.3f,%.3f,%.3f,%.1f,%.3f,%.2f,%.3f,%.3f,%.5f,%.3f,%.3f,%.2f,%.1f,%.3f,%.3f,%.3f,%.1f,%.1f,%.3f,%.3f,%.1f,%.4f,%.3f,%.3f,%.3f,%.3f,%.3f,%.2f,%.2f,%.3f\n",
          presets?-(k+1):seed*100000+k, (int)c.mono,c.effectiveWidth,c.sideVsMidDb,c.tilt,c.spatialMix,c.spatialDepth,c.spatialRate,c.haasLeadMs,c.phaseSpread,c.delayMix,c.delayFeedback,c.delayDepthMs,c.delayRateHz,c.delayTimeMs,
          c.microMix,c.detuneCents,c.diffusion,c.stereoSeparation,c.lfoDepth,c.lfoRate,c.exciterMix,c.exciterDrive,c.addedHarmonicsDb,c.reverbWet,c.reverbSize,c.reverbDamping,c.predelayMs,
          c.exciterHighpassHz,c.exciterHarmonicBalance,c.exciterBrightness,getter("sfxAllpassFreq"),getter("delayCentre"),
          sm-inSM,corr,smM,dHF,dLev,tail,tailE,motion);
        fflush(stdout);
    }
}
