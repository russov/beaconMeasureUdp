#include <iostream>
#include <fstream>
#include "gpf.h"
//#include <memAlloc.h>
//#include <mvg.h>

using namespace std;
using namespace PF;

#define _MDEBUG_
//----------------------------------
// - Each pf object is a point cloud consisting of 'Ns' particles. 
// - Each particle has a weight value (scalar) 'w', a state vector
//   'x' of size 'Nx x 1' and a measurement vector 'z' of size 'Nz x 1'. 
// - Output of the pf object or a point cloud is the weighted average
//   of the states of all the particles.
// - A state-transition or process model is needed to find a new
//   states for the particle. A measurement model is need to find the
//   estimated measurements for a given state. 
// - The weights are modified based on the observation / likelihood
//   function.
// - The particles are re-sampled to maintain a healthy population of
//   particles within the cloud.

//-----------------------------
// Default constructor
pf::pf()
{
  itn_num = 0;
  Ns = 100;
  Nx = 1;
  Nz = 1;
  ss = SYSTEMATIC;

  cout << "This is the default constructor." << endl;
  cout << "Call with more arguments for different initialization" << endl;

  w.resize(Ns);
  xk.resize(Ns, std::vector<double>(Nx));
  xfk.resize(Nx);
  zk.resize(Ns, std::vector<double>(Nz));

  //long seed = time(NULL)*getpid();
  //r = gsl_rng_alloc(gsl_rng_rand48);
  //gsl_rng_set(r,seed);
}
//-----------------------------------------------
// Constructor
// Input: 
//  - ns : number of samples
//  - nx : size of state vector
//  - nz : size of the measurement vector
// ---------------------------------------------------
pf::pf(uint ns, uint nx, uint nz,sampling_strategy type)
{
  itn_num = 0;
  Nx = nx;                    // size of state vector     
  Ns = ns;                    // number of samples 
  Nz = nz;                    // size of each landmark vector

  ss = type;                  // Sampling Strategy

  w.resize(Ns);
  xk.resize(Ns, std::vector<double>(Nx));
  zk.resize(Ns, std::vector<double>(Nz));
  xfk.resize(Nx);


//  long seed = time(NULL)*getpid();
//  r = gsl_rng_alloc(gsl_rng_rand48);
//  gsl_rng_set(r,seed);
}
//--------------------------------------------
// Destructor
pf::~pf()
{
  //gsl_rng_free(r);
}
//-----------------------------------------
// Copy Constructor
// --------------------
pf::pf(const pf &o)
{
  itn_num = o.itn_num;
  Nx = o.Nx;                    // size of state vector     
  Ns = o.Ns;                    // number of samples 
  Nz = o.Nz;                    // size of each landmark vector

  w.resize(Ns);
  xk.resize(Ns, std::vector<double>(Nx));
  zk.resize(Ns, std::vector<double>(Nz));
  xfk.resize(Nx);

  for(uint i = 0; i < Ns; ++i)
  {
    w[i] = o.w[i];
    for(uint j = 0; j < Nx; ++j)
      xk[i][j] = o.xk[i][j];
    for(uint j = 0; j < Nz; ++j)
      zk[i][j] = o.zk[i][j];
  }

  //GSL random number generator
//  long seed = time(NULL)*getpid();
//  r = gsl_rng_alloc(gsl_rng_rand48);
//  gsl_rng_set(r,seed);
}
//----------------------------------------
// Initializing Particle Filter
void pf::initialize(uint k, double mean, double noise_sd)
{
  std::normal_distribution<double> distribution(0.0, noise_sd);

  itn_num = k;
  for(uint i = 0; i < Ns; ++i)
  {
    w[i] = 1.0/(double)Ns;

    for(uint j = 0; j < Nx; ++j)
      xk[i][j] = mean + distribution(generator); //(r, noise_sd);

    for(uint j = 0; j < Nz; ++j)
      zk[i][j] = mean + distribution(generator); //gsl_ran_gaussian(r, noise_sd);
  }
  itn_num++;
}
//---------------------------------------------------------
// Initialize PF: overloaded function
void pf::initialize(uint k, const std::vector<double> &wt, const std::vector<std::vector<double> > &xd)
{
  std::normal_distribution<double> distribution(0.0,1.0);

  itn_num = k;
  for(uint i = 0; i < Ns; ++i)
  {
    w.at(i) = wt.at(i);

    for(uint j = 0; j < Nx; ++j)
      xk[i][j] = xd[i][j];

    for(uint j = 0; j < Nz; ++j)
      zk[i][j] = distribution(generator); //gsl_ran_gaussian(r, 1.0);
  }
}
//---------------------------------------------
// Resampling
//-------------------------------------
// Re-sampling function
// strategy 0 - Weighted Random Sampling with Replacement
//          1 - Multinomial Re-sampling
//          2 - Systematic Re-sampling
//          3 - Residual Sampling  (Not implemented)
//          4 - Stratified Sampling (Not implemented)
//
//----------------------------------------
// Note: multinomial sampling is same as wrswr
void pf::resample()
{
  switch(ss)
  {
    case WRSWR: 
      wrswr(); // weighted random sampling with replacement
      for(size_t i = 0; i < Ns; ++i)
        w.at(i) = 1/(double)Ns;
      break;
    case MULTINOMIAL:
      multinomial_sampling(); // multinomial sampling
      for(size_t i = 0; i < Ns; ++i)
        w.at(i) = 1/(double)Ns;
      break;
    case SYSTEMATIC: // Systematic Re-Sampling
      systematic_resampling();
      for(size_t i = 0; i < Ns; ++i)
        w.at(i) = 1/(double)Ns;
      break;
    default:
      cerr << "Strategy not defined" << endl;
      exit(-1);
      break;
  }
}
//--------------------------------------------------------
// Weighted random sampling with replacement: WRSWR
// Last updated: May 22, 2012
// ------------------------------------------
void pf::wrswr()
{
  std::vector< std::vector<double> > tempx(Ns, std::vector<double>(Nx));

  std::uniform_real_distribution<double> distribution(0.0,1.0);

  double sum_w = 0.0;
  for(size_t i = 0; i < w.size(); ++i)
  {
    sum_w += w.at(i);
    tempx.at(i) = xk.at(i);  // copy of source xk

  }

  for(size_t i = 0; i < Ns; ++i)
  {
    double u = distribution(generator); //gsl_rng_uniform(r);  // generate a uniform random number between 0 and 1


    size_t j = 0;
    double c = w[0] / sum_w;
    while (u > c && j < Ns)
    {
      j = j + 1;
      c = c + w[j] / sum_w;
    }
    xk.at(i) = tempx.at(j);

  }


 /*
  size_t cnt = 0;
  while(cnt < Ns)
  {
    double total_w = 0.0;
    for(size_t i = 0; i < Ns; ++i)   // for each item in the src list
    {
      double u = gsl_rng_uniform(r)*sum_w;  // generate a random number between 0 & sum_w
      total_w += w[i];       // running total of weights
      if(total_w >= u && cnt < Ns)
      {
        //dest[cnt] = i;     // store the index
        xk.at(cnt) = tempx.at(i);  // new value of xk
        cnt++;
        //break;
        continue;
      }
    }
  } */
}
//-------------------------------------------------
// multinomial sampling
void pf::multinomial_sampling()
{
/*  std::vector<double> pw(Ns);
  std::vector< std::vector<double> > tempx(Ns, std::vector<double>(Nx));

  std::vector<uint> idx(Ns);
  double sum_w = 0.0;
  for(size_t i = 0; i < Ns; ++i)
  {
    sum_w += w[i];
    tempx.at(i) = xk.at(i); //original copy of xk
  }

  //probability
  for(size_t i = 0; i < Ns; ++i)
    pw[i] = w[i] / sum_w;

  gsl_ran_multinomial(r, Ns, Ns, pw.data(), idx.data());  

  int j = 0;
  uint cnt;
  for(uint i = 0; i < Ns; ++i)
  {
    cnt = idx[i];
    while(cnt > 0)
    {
      xk.at(j) = tempx.at(i);
      //dest[j] = src[i];
      j = j + 1;
      cnt = cnt-1;
    }
  }*/
}
//-------------------------------------------------------------------------
// Systematic Resampling
// Date: May 21, 2013
//-----------------------------------------
void pf::systematic_resampling()
{
  std::vector<std::vector<double> > tempx(Ns, std::vector<double>(Nx));

  std::uniform_real_distribution<double> distribution(0.0,1.0);

  double sum_w = 0.0;
  for(size_t i = 0; i < Ns; ++i)
  {
    sum_w += w[i];
    tempx.at(i) = xk.at(i);
  }

  double u;
  double u1 = distribution(generator) / Ns; //gsl_rng_uniform(r) / Ns;
  size_t i = 0;
  double C = w[0]/sum_w;
  size_t j = 0;
  size_t k = 0;

  for(uint k = 0; k < Ns; ++k)
  {
    u = u1 + (1.0/(double) Ns) * k; 

    while(u > C && i < Ns)
    {
      i = i + 1;
      C = C + w[i]/sum_w;
    }
    while(j <= i && i < Ns) 
    {
      xk.at(j) = tempx.at(i);  // repeated-items
      j = j + 1;
    }
  }
}
//------------------------------------
// Particle Filter Update 
void pf::particleFilterUpdate(void (*pmodel)(std::vector<double> &x, const std::vector<double> &xprev, const std::vector<double> &step, void* data),  // Process Model
    void (*omodel)(std::vector<double> &z, const std::vector<double> &x,  void *data),  // Observation Model
    double(*likelihood)(const std::vector<double> &z, const std::vector<double> &zhat, void *data),
    const std::vector<double> &z,
    const std::vector<double> &Step,
    uint resample_size)
{
  double sum_wt = 0.0;
  //ofstream f("rdata.txt");
  //for each sample
  for(uint i = 0; i < Ns; ++i)
  {
    // a priori pdf 
    // x'(t) =  x(t-1) (+) ut , where (+) is the pose compounding
    // operator which is simply the motion model

    pmodel(xk[i], xk[i], Step, (void*)&this->generator);   // xk ~ P(xk | xkm1): Hypothesis - State Transition Model
    omodel(zk[i], xk[i], (void*)&this->generator);   // zk ~ P(zk | xk): Observation Model

    // update the weights based on likelihood function
    w[i] = w[i] * likelihood(z, zk[i], (void*)&this->generator); // P(zk, xk) for a given xk
      

    sum_wt += w[i];         // Total weight

    /*for(uint j = 0; j < Nx; ++j)
      f << xk[i][j] << "\t" ;
    for(uint j = 0; j < Nz; ++j)
      f << zk[i][j] << "\t" ;
    f << w[i] << endl; */
  }
  //f.close();
  //getchar();


  //Filtered Output
  for(uint i =0; i < Nx; ++i)
    xfk[i] = 0.0;

  double sum_wt2 = 0.0;
  for(uint i = 0; i < Ns; ++i)
  {
    w[i] = w[i] / sum_wt;

    sum_wt2 += w[i]* w[i];           

    for(uint j = 0; j < Nx; ++j)
      xfk[j] += w[i] * xk[i][j];
  }

  // normalized weights are used for computing Neff.
  this->Neff = 1.0/ sum_wt2;      

  if(resample_size > 0)
  { 
    if(Neff < resample_size)
      this->resample();
  }

  itn_num++; // update iteration count
}//eof
//------------------------------------------------------------
/*
void pfupdate(
    void (*pmodel)(std::vector<double> &x, const std::vector<double> &xprev, void* data), // Process Model 
    void (*omodel)(std::vector<double> &z, const std::vector<double> &x, void* data), // Observation Model 
    double(*likelihood)(const std::vector<double> &z, const std::vector<double> &xhat, void *data),
    const std::vector<std::vector<double> > &z,   //Observation Model
    uint resample_size = 0);  //default option: no resample 
{
  double sum_wt = 0.0;
  double sum_wt2 = 0.0;
  //for each sample
  for(uint i = 0; i < Ns; ++i)
  {
    // a priori pdf 
    // x'(t) =  x(t-1) (+) ut , where (+) is the pose compounding
    // operator which is simply the motion model

    pmodel(xk[i], xk[i], (void*)this->r);   // xk ~ P(xk | xkm1): Hypothesis - State Transition Model
    omodel(zk[i], xk[i], (void*)this->r);   // zk ~ P(zk | xk): Observation Model

    // update the weights based on likelihood function
    w[i] = w[i] * likelihood(z[i], zk[i], (void*)this->r); // P(zk, xk) for a given xk

    sum_wt += w[i];         // Total weight
    sum_wt2 += w[i]* w[i];           
  }

  //Filtered Output
  for(uint i =0; i < Nx; ++i)
    xfk[i] = 0.0;

  for(uint i = 0; i < Ns; ++i)
  {
    w[i] = w[i] / sum_wt;

    for(uint j = 0; j < Nx; ++j)
      xfk[j] += w[i] * xk[i][j];
  }

  this->Neff = 1.0/ sum_wt2;

  //cout << Neff << "\t" << sum_wt2 << endl;
  //getchar();

  if(resample_size > 0)
  { 
    if(Neff < resample_size)
      this->resample();
  }

  itn_num++; // update iteration count
}//eof

*/

//----------------------------------------------
void pf::filterOutput(std::vector<double> &xf)
{
  for(uint i = 0; i < Nx; ++i)
    xf[i] = this->xfk[i];
}
//-----------------------------------------
double pf::getEffectivePopulation()
{
  return Neff;
}
//---------------------------------------------
double pf::getParticleState(std::vector<double> &x, std::vector<double> &z, uint pfidx)
{
  if(pfidx < 0 || pfidx > Ns-1)
  {
    cerr << __LINE__ << "particle index out of range [0, Ns-1]." << endl;
    exit(-1);
  }
  else
  {
    for(uint i = 0; i < Nx; ++i)
      x[i] = this->xk[pfidx][i];
    //for(uint i = 0; i < Nz; ++i)
    //  z[i] = this->zk[pfidx][i];

    return w[pfidx];
  }
}
//--------------------------------------------
void pf::print_states()
{
  cout << "\n -------------------------" << endl;
  cout << "x = " << endl;
  for(uint i = 0; i < Ns; ++i)
  {
    for(uint j = 0; j < Nx; ++j)
      cout << this->xk.at(i).at(j) << "\t";
    cout << endl;
  }
  cout << "-------------" << endl;
}
//---------------
void pf::display_array(double **xv, uint row, uint col)
{
  cout << "\n-----------------" << endl;
  for(uint i = 0; i < row; ++i)
  {
    for(uint j = 0; j < col; ++j)
      cout << xv[i][j] << "\t";
    cout << endl;
  }
  cout << "----------------" << endl;
}
//--------------------------------
uint pf::getItnNum()
{
  return itn_num;
}

