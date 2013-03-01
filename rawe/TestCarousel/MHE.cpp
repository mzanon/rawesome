/*
 *    This file is part of ACADO Toolkit.
 *
 *    ACADO Toolkit -- A Toolkit for Automatic Control and Dynamic Optimization.
 *    Copyright (C) 2008-2009 by Boris Houska and Hans Joachim Ferreau, K.U.Leuven.
 *    Developed within the Optimization in Engineering Center (OPTEC) under
 *    supervision of Moritz Diehl. All rights reserved.
 *
 *    ACADO Toolkit is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    ACADO Toolkit is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with ACADO Toolkit; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


 /**
 *    SINGLE POWER KITE START-UP WITH PROPELLER
 *    CARTESIAN COORDINATES (ODE FORMULATION)
 *    MARCH 2013 SEBASTIEN GROS & MARIO ZANON & GREG HORN, HIGHWIND, OPTEC
 *    SEBASTIEN GROS & MARIO ZANON & GREG HORN
 */


#include <acado_toolkit.hpp>
#include <mhe_export.hpp>



int main( int argc, char * const argv[] ){


	//======================================================================
	// PARSE THE INPUT ARGUMENTS:
	// ----------------------------------
	double IC[5];
	
	/* arguments are passed to the main function by string
	 *  there are 'int argc' arguments
	 *		the first one is the name of the program
	 *		the next ones are arguments passed in the call like
	 *			program number1 number2
	 *  in this stupid simple parser, we directly read doubles from the arguments
	 */
	 
	int i=1;
	while (i < argc) {
		// load the i-th string argument passed to the main function
		char* input = argv[i];
		// parse the string to a double value
		IC[i-1] = atof(input);
		i++;
	}
	
	cout << "Initial Conditions" << endl;
	cout << "------------" << endl;
	for (i = 0; i < argc-1; ++i) {
		cout << i+1 << ":\t" << IC[i] << endl;
	}
	//======================================================================	
	
	double Ncvp  = IC[1];
	double Tc    = IC[2];
	int MaxSteps = IC[3];
	 
	 
	USING_NAMESPACE_ACADO

	
	#include "model.cpp"


	// ===============================================================
	//                        END OF MODEL EQUATIONS
	// ===============================================================
	
	
	// DEFINE A MEASUREMENT FUNCTION:
	// ------------------------------

	// The camera measurement is given by: [u;v;s;] = Proj*Rp*Translation(pos)*Rotation(q)*pos_marker_body;

	IntermediateState Rotation(4,4);
	Rotation = eye(4);
	Rotation(0,0) = e11;
	Rotation(0,1) = e12;
	Rotation(0,2) = e13;
	Rotation(1,0) = e21;
	Rotation(1,1) = e22;
	Rotation(1,2) = e23;
	Rotation(2,0) = e31;
	Rotation(2,1) = e32;
	Rotation(2,2) = e33;

	IntermediateState Translation(4,4);
	Translation = eye(4);
	Translation(0,3) = x;//pos[0];
	Translation(1,3) = y;//pos[1];
	Translation(2,3) = z;//pos[2];
	
	
	Matrix RPC1 = readFromFile( "CameraCalibration/RPC1.txt" );
	Matrix RPC2 = readFromFile( "CameraCalibration/RPC2.txt" );
	Matrix ProjC1 = readFromFile( "CameraCalibration/PC1.txt" );
	Matrix ProjC2 = readFromFile( "CameraCalibration/PC2.txt" );
	Matrix pos_marker_body1 = readFromFile( "CameraCalibration/pos_marker_body1.txt" );
	Matrix pos_marker_body2 = readFromFile( "CameraCalibration/pos_marker_body2.txt" );
	Matrix pos_marker_body3 = readFromFile( "CameraCalibration/pos_marker_body3.txt" );
	
	
	Matrix RpC1Temp(4,4), RpC2Temp(4,4);
	RpC1Temp = eye(4);
	RpC1Temp(0,0) = RPC1(0,0);
	RpC1Temp(0,1) = RPC1(1,0);
	RpC1Temp(0,2) = RPC1(2,0);
	RpC1Temp(1,0) = RPC1(0,1);
	RpC1Temp(1,1) = RPC1(1,1);
	RpC1Temp(1,2) = RPC1(2,1);
	RpC1Temp(2,0) = RPC1(0,2);
	RpC1Temp(2,1) = RPC1(1,2);
	RpC1Temp(2,2) = RPC1(2,2);
	
	RpC2Temp = eye(4);
	RpC2Temp(0,0) = RPC2(0,0);
	RpC2Temp(0,1) = RPC2(1,0);
	RpC2Temp(0,2) = RPC2(2,0);
	RpC2Temp(1,0) = RPC2(0,1);
	RpC2Temp(1,1) = RPC2(1,1);
	RpC2Temp(1,2) = RPC2(2,1);
	RpC2Temp(2,0) = RPC2(0,2);
	RpC2Temp(2,1) = RPC2(1,2);
	RpC2Temp(2,2) = RPC2(2,2);
	
	RpC1Temp(0,3) = -(RPC1(0,0)*RPC1(0,3) + RPC1(1,0)*RPC1(1,3) + RPC1(2,0)*RPC1(2,3));
	RpC1Temp(1,3) = -(RPC1(0,1)*RPC1(0,3) + RPC1(1,1)*RPC1(1,3) + RPC1(2,1)*RPC1(2,3));
	RpC1Temp(2,3) = -(RPC1(0,2)*RPC1(0,3) + RPC1(1,2)*RPC1(1,3) + RPC1(2,2)*RPC1(2,3));
	
	RpC2Temp(0,3) = -(RPC2(0,0)*RPC2(0,3) + RPC2(1,0)*RPC2(1,3) + RPC2(2,0)*RPC2(2,3));
	RpC2Temp(1,3) = -(RPC2(0,1)*RPC2(0,3) + RPC2(1,1)*RPC2(1,3) + RPC2(2,1)*RPC2(2,3));
	RpC2Temp(2,3) = -(RPC2(0,2)*RPC2(0,3) + RPC2(1,2)*RPC2(1,3) + RPC2(2,2)*RPC2(2,3));
	
	Matrix RpC1Full(4,4), RpC2Full(4,4);
	RpC1Full = RpC1Temp;
	RpC2Full = RpC2Temp;
	
	Matrix RpC1(3,4), RpC2(3,4);
	RpC1(0,0) = RpC1Full(0,0); 	RpC1(0,1) = RpC1Full(0,1); 	RpC1(0,2) = RpC1Full(0,2); 	RpC1(0,3) = RpC1Full(0,3); 
	RpC1(1,0) = RpC1Full(1,0); 	RpC1(1,1) = RpC1Full(1,1); 	RpC1(1,2) = RpC1Full(1,2); 	RpC1(1,3) = RpC1Full(1,3); 
	RpC1(2,0) = RpC1Full(2,0); 	RpC1(2,1) = RpC1Full(2,1); 	RpC1(2,2) = RpC1Full(2,2); 	RpC1(2,3) = RpC1Full(2,3); 
	
	RpC2(0,0) = RpC2Full(0,0); 	RpC2(0,1) = RpC2Full(0,1); 	RpC2(0,2) = RpC2Full(0,2); 	RpC2(0,3) = RpC2Full(0,3); 
	RpC2(1,0) = RpC2Full(1,0); 	RpC2(1,1) = RpC2Full(1,1); 	RpC2(1,2) = RpC2Full(1,2); 	RpC2(1,3) = RpC2Full(1,3); 
	RpC2(2,0) = RpC2Full(2,0); 	RpC2(2,1) = RpC2Full(2,1); 	RpC2(2,2) = RpC2Full(2,2); 	RpC2(2,3) = RpC2Full(2,3); 
	
	IntermediateState uvsC1M1(3,1), uvsC2M2(3,1), uvsC1M3(3,1), uvsC2M1(3,1), uvsC1M2(3,1), uvsC2M3(3,1);
	uvsC1M1 = ProjC1*RpC1*Translation*Rotation*pos_marker_body1;
	uvsC1M2 = ProjC1*RpC1*Translation*Rotation*pos_marker_body2;
	uvsC1M3 = ProjC1*RpC1*Translation*Rotation*pos_marker_body3;
	uvsC2M1 = ProjC2*RpC2*Translation*Rotation*pos_marker_body1;
	uvsC2M2 = ProjC2*RpC2*Translation*Rotation*pos_marker_body2;
	uvsC2M3 = ProjC2*RpC2*Translation*Rotation*pos_marker_body3;
	
	IntermediateState uvC1M1(2,1), uvC2M2(2,1), uvC1M3(2,1), uvC2M1(2,1), uvC1M2(2,1), uvC2M3(2,1);
	uvC1M1(0,0) = uvsC1M1(0,0)/uvsC1M1(2,0);	uvC1M1(1,0) = uvsC1M1(1,0)/uvsC1M1(2,0);
	uvC1M2(0,0) = uvsC1M2(0,0)/uvsC1M2(2,0);	uvC1M2(1,0) = uvsC1M2(1,0)/uvsC1M2(2,0);
	uvC1M3(0,0) = uvsC1M3(0,0)/uvsC1M3(2,0);	uvC1M3(1,0) = uvsC1M3(1,0)/uvsC1M3(2,0);
	
	uvC2M1(0,0) = uvsC2M1(0,0)/uvsC2M1(2,0);	uvC2M1(1,0) = uvsC2M1(1,0)/uvsC2M1(2,0);
	uvC2M2(0,0) = uvsC2M2(0,0)/uvsC2M2(2,0);	uvC2M2(1,0) = uvsC2M2(1,0)/uvsC2M2(2,0);
	uvC2M3(0,0) = uvsC2M3(0,0)/uvsC2M3(2,0);	uvC2M3(1,0) = uvsC2M3(1,0)/uvsC2M3(2,0);
	
	
	Matrix RIMU = readFromFile( "IMU/RIMU.txt" );
// 	RIMU = eye(3);
	
	IntermediateState aE(3,1), aEend(3,1);
	aE(0,0) = ddxIMU;
	aE(1,0) = ddyIMU;
	aE(2,0) = ddzIMU;
	aEend(0,0) = ddxIMUend;
	aEend(1,0) = ddyIMUend;
	aEend(2,0) = ddzIMUend;
	
	IntermediateState Omega(3,1);
	Omega(0,0) = w1;
	Omega(1,0) = w2;
	Omega(2,0) = w3;
	
	IntermediateState aSHIFT(3,1);
	aSHIFT(0,0) = XIMU3*dw2 - XIMU2*dw3 + w2*(XIMU2*w1 - XIMU1*w2) + w3*(XIMU3*w1 - XIMU1*w3);
	aSHIFT(1,0) = XIMU1*dw3 - XIMU3*dw1 - w1*(XIMU2*w1 - XIMU1*w2) + w3*(XIMU3*w2 - XIMU2*w3);
	aSHIFT(2,0) = XIMU2*dw1 - XIMU1*dw2 - w1*(XIMU3*w1 - XIMU1*w3) - w2*(XIMU3*w2 - XIMU2*w3);
	
	IntermediateState aIMU, aIMUend, wIMU;
	aIMU = RIMU*aE;
	aIMUend = RIMU*aEend;
	wIMU = RIMU*Omega;
	
	Function CostLSQend;
	
	CostLSQend << wIMU(0,0);
	CostLSQend << wIMU(1,0);
	CostLSQend << wIMU(2,0);
	CostLSQend << aIMUend(0,0);
	CostLSQend << aIMUend(1,0);
	CostLSQend << aIMUend(2,0);
	CostLSQend << r;
	CostLSQend << delta;
	CostLSQend << ur;
	CostLSQend << up;
	
	
	// COST FUNCTION
	//---------------
	Function CostLSQx;
	
	int j = 0;
	

	CostLSQx << uvC1M1(0,0);
	CostLSQx << uvC1M1(1,0);
	CostLSQx << uvC1M2(0,0);
	CostLSQx << uvC1M2(1,0);
	CostLSQx << uvC1M3(0,0);
	CostLSQx << uvC1M3(1,0);
	CostLSQx << uvC2M1(0,0);
	CostLSQx << uvC2M1(1,0);
	CostLSQx << uvC2M2(0,0);
	CostLSQx << uvC2M2(1,0);
	CostLSQx << uvC2M3(0,0);
	CostLSQx << uvC2M3(1,0);
	CostLSQx << wIMU(0,0);
	CostLSQx << wIMU(1,0);
	CostLSQx << wIMU(2,0);
	CostLSQx << aIMU(0,0);
	CostLSQx << aIMU(1,0);
	CostLSQx << aIMU(2,0);
	CostLSQx << r;
	CostLSQx << delta;
	CostLSQx << ur;
	CostLSQx << up;
	
	Function CostLSQu;
	CostLSQu << dddelta;
	CostLSQu << ddr;
	CostLSQu << dur;
	CostLSQu << dup;

	Expression ExprCostLSQx;
	CostLSQx.getExpression(ExprCostLSQx);
	Expression ExprCostLSQu;
	CostLSQu.getExpression(ExprCostLSQu);
	
	Function CostLSQ;
	CostLSQ << ExprCostLSQx;
	CostLSQ << ExprCostLSQu;
	
	
	cout << "f dim: " << f.getDim() << endl;
	cout << "CostLSQend dim: " << CostLSQend.getDim() << endl;
	cout << "CostLSQend #u: " << CostLSQend.getNU() << endl;
	cout << "CostLSQ dim: " << CostLSQ.getDim() << endl;
	cout << "CostLSQ  #u: " << CostLSQ.getNU() << endl;
	
	
	
    // DEFINE AN OPTIMAL CONTROL PROBLEM:
    // ----------------------------------
	OCP ocp( 0.0, Tc, Ncvp );
	
	ExportVariable QQ, QQend;
	
	ocp.minimizeLSQ(QQ, CostLSQ);
	ocp.minimizeLSQEndTerm(QQend, CostLSQend);
	
	
	
	ocp.subjectTo( f );

	
	// TERMINAL CONSTRAINTS
	// ---------------------------------
	ocp.subjectTo( AT_END, ConstR1 == 0 );
	ocp.subjectTo( AT_END, ConstR2 == 0 );
	ocp.subjectTo( AT_END, ConstR3 == 0 );
	ocp.subjectTo( AT_END, ConstR4 == 0 );
	ocp.subjectTo( AT_END, ConstR5 == 0 );
	ocp.subjectTo( AT_END, ConstR6 == 0 );
	
	ocp.subjectTo( AT_END,  Const == 0 );
	ocp.subjectTo( AT_END, dConst == 0 );

	// CONTROL
	double AccRate = 30*PI/180;
	ocp.subjectTo( -AccRate <= dddelta <= AccRate );
// 	ocp.subjectTo(  -0.5 <= ddr <= 0.5 );
    
	ocp.subjectTo(  -1 <= up  <= 1 );
	ocp.subjectTo(  -3.2767 <= ur  <= 3.2767 );
// 	ocp.subjectTo( -10 <= dup <= 10 );
// 	ocp.subjectTo( -10 <= dur <= 10 );
    
// 	ocp.subjectTo( -0.2 <= up <= 0.2 );
// 	ocp.subjectTo( -0.2 <= ur <= 0.2 );
// 	ocp.subjectTo( -20.0 <= dup <= 20.0 );
// 	ocp.subjectTo( -20.0 <= dur <= 20.0 );
	
	
	
	printf("EXPORTING LINUX/QPOASES CODE\n");
	MHEexport mhe( ocp );

	mhe.set( HESSIAN_APPROXIMATION, GAUSS_NEWTON    );
	mhe.set( DISCRETIZATION_TYPE,   MULTIPLE_SHOOTING );
	//	mhe.set( DISCRETIZATION_TYPE,   SINGLE_SHOOTING );
	
	// 	mhe.set( INTEGRATOR_TYPE,             INT_RK4    );
	// 	mhe.set( NUM_INTEGRATOR_STEPS,        100              );
	
	mhe.set( INTEGRATOR_TYPE,             INT_IRK_GL2     );
	mhe.set( NUM_INTEGRATOR_STEPS,        30             );
	
	mhe.set( IMPLICIT_INTEGRATOR_NUM_ITS, 3				);
	mhe.set( IMPLICIT_INTEGRATOR_NUM_ITS_INIT, 0		);
	mhe.set( LINEAR_ALGEBRA_SOLVER,		  HOUSEHOLDER_QR );
	mhe.set( UNROLL_LINEAR_SOLVER,        NO	      );
	mhe.set( IMPLICIT_INTEGRATOR_MODE, IFTR );
	
	mhe.set(SPARSE_QP_SOLUTION, CONDENSING);
	
	mhe.set( QP_SOLVER,             QP_QPOASES      );
	mhe.set( HOTSTART_QP,           YES              );
//     mhe.set( GENERATE_TEST_FILE,    YES              );
	// 	mpc.set( GENERATE_MAKE_FILE,    YES              );
	
	mhe.set( CG_USE_C99,    YES              );
	
	mhe.set(PRINTLEVEL, HIGH);
	
// 	mhe.set( CG_USE_VARIABLE_WEIGHTING_MATRIX, YES);

	
	
	mhe.exportCode( "code_export_MHE" );
	mhe.printDimensionsQP();

    return 0;
}


