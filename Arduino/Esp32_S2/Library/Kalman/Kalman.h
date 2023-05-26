/*
 * Implement the Kalman filter corresponding to the linear problem
 *    x_k = F*x_{k-1} + B*u_k + q_k   (evolution model)
 *    y_k = H*x_k + r_k               (measure)
 *
 * with the matrices and vectors
 *    x [output] [size=Nstate]          Estimated state vector
 *    F [input]  [size=(Nstate,Nstate)] Free evolution of the state vector
 *    B [input]  [size=(Nstate,Ncom)]   [optional] Command vector acting on state
 *    Q [input]  [size=(Nstate,Nstate)] Model covariance acting as (1/inertia)
 *    y [input]  [size=Nobs]            Observed (measured) data from sensors
 *    H [input]  [size=(Nobs,Nstate)]   Observation matrix
 *    R [input]  [size=(Nobs,Nobs)]     Measurement noise covariance matrix
 *
 * Many attributes are public, so you might modify them as you wish.
 * However be careful since modification of attributes (especially 'P' and 'x')
 * might lead to unconsistant results.
 * Use the 'getxcopy' method to get a copy of the 'x' state vector.
 *
 * Requires:
 *  BasicLinearAlgebra  https://github.com/tomstewart89/BasicLinearAlgebra
 *
 * License:
 *  See the LICENSE file
 *
 * Author:
 *  R.JL. Fétick
 *
 */

#ifndef Kalman_h
#define Kalman_h

#include <BasicLinearAlgebra.h>

#include <Arduino.h>

#define KALMAN_CHECK true
#define KALMAN_VERBOSE false

using namespace BLA;

/**********    PARTICULAR MATRIX DEFINITION    **********/
// These matrices require less memory than normal ones.
// If your matrices are symmetric or triangular, use these ones for Arduino SRAM saving

// Symmetric (i=row*cols+col)
template<int dim, class ElemT> struct Symmetric{
  mutable ElemT m[dim*(dim+1)/2];
  typedef ElemT elem_t;
  ElemT &operator()(int row, int col) const{
    static ElemT dummy;
    if(col < dim && row < dim){
      if(col < row){ // swap row and col
        ElemT temp = row;
        row = col;
        col = temp;
      }
      return m[(2*dim-row+1)*row/2+col-row];
    }else
      return (dummy = 0);
  }
};

// SkewSymmetric (i=row*cols+col)
template<int dim, class ElemT> struct SkewSymmetric{
  mutable ElemT m[dim*(dim+1)/2];
  typedef ElemT elem_t;
  ElemT &operator()(int row, int col) const{
    static ElemT dummy;
    if(col < dim && row < dim && row!=col){
		if(col < row){
			return -m[(2*dim-col+1)*col/2+row-col];
		}else{
			return m[(2*dim-row+1)*row/2+col-row];
		}
    }else{
      return (dummy = 0);
	}
  }
};

// Triangular sup (i=row*cols+col)
template<int dim, class ElemT> struct TriangularSup{
  mutable ElemT m[dim*(dim+1)/2];
  typedef ElemT elem_t;
  ElemT &operator()(int row, int col) const{
    static ElemT dummy;
    if(col < dim && row < dim && col>=row){
      return m[(2*dim-row+1)*row/2+col-row];
    }else
      return (dummy = 0);
  }
};

// Triangular inf (i=row*cols+col)
template<int dim, class ElemT> struct TriangularInf{
  mutable ElemT m[dim*(dim+1)/2];
  typedef ElemT elem_t;
  ElemT &operator()(int row, int col) const{
    static ElemT dummy;
    if(col < dim && row < dim && row>=col){
      return m[(2*dim-col+1)*col/2+row-col];
    }else
      return (dummy = 0);
  }
};

// Diagonal template comes from Tom Stewart (BasicLinearAlgebra)
template<int dim, class ElemT> struct Diagonal{
    mutable ElemT m[dim];
    // The only requirement on this class is that it implement the () operator like so:
    typedef ElemT elem_t;

    ElemT &operator()(int row, int col) const
    {
        static ElemT dummy;
        // If it's on the diagonal and it's not larger than the matrix dimensions then return the element
        if(row == col && row < dim)
            return m[row];
        else
            return (dummy = 0);
    }
};



/**********      CLASS DEFINITION      **********/

// Last arg of template allows to eventually define Symmetric, AntiSymmetric, TriangularSup or TriangularInf matrices for memory saving
template<int Nstate, int Nobs, int Ncom = 0, class MemF = Array<Nstate,Nstate,float> >
class KALMAN{
  private:
    void _update(BLA::Matrix<Nobs> obs, BLA::Matrix<Nstate> comstate);
    BLA::Identity<Nstate,Nstate> Id; // Identity matrix
  public:
	//INPUT MATRICES			
    BLA::Matrix<Nstate,Nstate,MemF> F; // time evolution matrix
    BLA::Matrix<Nobs,Nstate> H; // observation matrix
    BLA::Matrix<Nstate,Ncom> B; // Command matrix (optional)
    BLA::Matrix<Nstate,Nstate, Symmetric<Nstate,float> > Q; // model noise covariance matrix
    BLA::Matrix<Nobs,Nobs, Symmetric<Nobs,float> > R; // measure noise covariance matrix
	//OUTPUT MATRICES
    BLA::Matrix<Nstate,Nstate, Symmetric<Nstate,float> > P; // posterior covariance (do not modify, except to init!)
    BLA::Matrix<Nstate> x; // state vector (do not modify, except to init!)

    int status; // 0 if Kalman filter computed correctly

    // UPDATE FILTER WITH OBSERVATION
    void update(BLA::Matrix<Nobs> obs);

    // UPDATE FILTER WITH OBSERVATION and COMMAND
    void update(BLA::Matrix<Nobs> obs, BLA::Matrix<Ncom> com);

    // CONSTRUCTOR
  	KALMAN<Nstate,Nobs,Ncom,MemF>();

    // GETTER on X (copy vector to avoid eventual user modifications)
    BLA::Matrix<Nstate> getxcopy();

};

/**********      PRIVATE IMPLEMENTATION of UPDATE      **********/

template <int Nstate, int Nobs, int Ncom, class MemF>
void KALMAN<Nstate,Nobs,Ncom,MemF>::_update(BLA::Matrix<Nobs> obs, BLA::Matrix<Nstate> comstate){
  if(KALMAN_CHECK){
    for(int i=0;i<Nobs;i++){
      if(isnan(obs(i)) || isinf(obs(i))){
        if(KALMAN_VERBOSE){Serial.println(F("KALMAN:ERROR: observation has nan or inf values"));}
        status = 1;
        return;
      }
    }
  }
  BLA::Matrix<Nobs,Nobs> S;
  BLA::Matrix<Nstate,Nobs> K; // Kalman gain matrix
  // UPDATE
  this->x = this->F * this->x + comstate;
  this->P = this->F * this->P * (~ this->F) + this->Q;
  // ESTIMATION
  S = this->H * this->P * (~ this->H) + this->R;
  bool is_nonsingular = Invert(S); // inverse inplace (S <- S^{-1})
  K = P*(~H)*S;
  if(is_nonsingular){
    this->x += K*(obs - this->H * this->x); // K*y
    this->P = (this->Id - K * this->H)* this->P;
    if(KALMAN_CHECK){
      for(int i=0;i<Nstate;i++){
        if(isnan(this->x(i)) || isinf(this->x(i))){
          if(KALMAN_VERBOSE){Serial.println(F("KALMAN:ERROR: estimated vector has nan or inf values"));}
          status = 1;
          return;
        }
      }
    }
  }else{
    if(KALMAN_VERBOSE){Serial.println(F("KALMAN:ERROR: could not invert S matrix. Try to reset P matrix."));}
    this->P.Fill(0.0); // try to reset P. Better strategy?
    //K.Fill(0.0);
  }
};


/**********      UPDATE with OBS & COM      **********/
template <int Nstate, int Nobs, int Ncom, class MemF>
void KALMAN<Nstate,Nobs,Ncom,MemF>::update(BLA::Matrix<Nobs> obs, BLA::Matrix<Ncom> com){
  if(KALMAN_CHECK){
    for(int i=0;i<Ncom;i++){
      if(isnan(com(i)) || isinf(com(i))){
        if(KALMAN_VERBOSE){Serial.println(F("KALMAN:ERROR: command has nan or inf values"));}
        status = 1;
        return;
      }
    }
  }
  _update(obs,this->B *com);
};

/**********      UPDATE with OBS      **********/

template <int Nstate, int Nobs, int Ncom, class MemF>
void KALMAN<Nstate,Nobs,Ncom,MemF>::update(BLA::Matrix<Nobs> obs){
	BLA::Zeros<Nstate> NULLCOMSTATE;
  _update(obs,NULLCOMSTATE);
};

/**********      CONSTRUCTOR      **********/

template <int Nstate, int Nobs, int Ncom, class MemF>
KALMAN<Nstate,Nobs,Ncom,MemF>::KALMAN(){
  if(KALMAN_VERBOSE){
    Serial.println(F("KALMAN:INFO: Initialize filter"));
    if((Nstate<=1)||(Nobs<=1)){
      Serial.println(F("KALMAN:ERROR: 'Nstate' and 'Nobs' must be > 1"));
    }
  }
  this->P.Fill(0.0);
  this->x.Fill(0.0);
};

/**********      GETXCOPY      **********/

template <int Nstate, int Nobs, int Ncom, class MemF>
BLA::Matrix<Nstate> KALMAN<Nstate,Nobs,Ncom,MemF>::getxcopy(){
  BLA::Matrix<Nstate> out;
  for(int i=0;i<Nstate;i++){
    out(i) = this->x(i);
  }
  return out;
};


#endif
