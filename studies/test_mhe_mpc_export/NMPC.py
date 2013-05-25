import rawe
import casadi as C

intOpts = rawe.RtIntegratorOptions()
intOpts['INTEGRATOR_TYPE'] = 'INT_IRK_GL2'
intOpts['NUM_INTEGRATOR_STEPS'] = 40
intOpts['IMPLICIT_INTEGRATOR_NUM_ITS'] = 3
intOpts['IMPLICIT_INTEGRATOR_NUM_ITS_INIT'] = 0
intOpts['LINEAR_ALGEBRA_SOLVER'] = 'HOUSEHOLDER_QR'
intOpts['UNROLL_LINEAR_SOLVER'] = False
intOpts['IMPLICIT_INTEGRATOR_MODE'] = 'IFTR'
    
def makeNmpc(dae,N,dt):
    mpc = rawe.Ocp(dae, N=N, ts=dt)
    
    ocpOpts = rawe.OcpExportOptions()
    ocpOpts['HESSIAN_APPROXIMATION'] = 'GAUSS_NEWTON'
    ocpOpts['DISCRETIZATION_TYPE'] = 'MULTIPLE_SHOOTING'
    ocpOpts['QP_SOLVER'] = 'QP_QPOASES'
    ocpOpts['HOTSTART_QP'] = False
    ocpOpts['SPARSE_QP_SOLUTION'] = 'CONDENSING'
#   ocpOpts['SPARSE_QP_SOLUTION'] = 'FULL_CONDENSING_U2'
#   ocpOpts['AX_NUM_QP_ITERATIONS'] = '30'
    ocpOpts['FIX_INITIAL_STATE'] = True
               
    mpc.minimizeLsq(C.veccat([mpc['x'],mpc['v'],mpc['u']]))
    mpc.minimizeLsqEndTerm(C.veccat([mpc['x'],mpc['v']]))

    cgOpts = {'CXX':'g++', 'CC':'gcc'}
    return rawe.OcpRT(mpc, ocpOptions=ocpOpts, integratorOptions=intOpts,
                       codegenOptions=cgOpts)
