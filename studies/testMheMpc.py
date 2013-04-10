import matplotlib.pyplot as plt

import rawe
import casadi as C

# specify the dae
def makeDae():
    dae = rawe.dae.Dae()

    [pos,vel] = dae.addX( ["pos","vel"] )
    force = dae.addU( "force" )
    
    dae.setResidual([dae.ddt('pos') - vel,
                     dae.ddt('vel') - (force - 0*3.0*pos - 0*0.2*vel)])
    return dae

def makeMpc(dae, N, ts):
    mpc = rawe.ocp.Ocp(dae, N=N, ts=ts)
    mpc.constrain(-0.5, '<=', mpc['force'], '<=', 0.5)

    mpc.minimizeLsq([mpc['pos'],mpc['vel'],mpc['force']])
    mpc.minimizeLsqEndTerm([mpc['pos'],mpc['vel']])

#    cgOptions = {'CXX':'clang++', 'CC':'clang'}
    cgOptions = {'CXX':'g++', 'CC':'gcc'}
#    cgOptions = {'CXX':'icpc', 'CC':'icc'}
    acadoOptions = [("HESSIAN_APPROXIMATION",     "GAUSS_NEWTON"),
                    ("DISCRETIZATION_TYPE",       "MULTIPLE_SHOOTING"),
                    ("INTEGRATOR_TYPE",           "INT_IRK_RIIA3"),
                    ("NUM_INTEGRATOR_STEPS",      str(N*5)),
                    ("LINEAR_ALGEBRA_SOLVER",     "GAUSS_LU"),
                    ("SPARSE_QP_SOLUTION",        "FULL_CONDENSING"),
#                    ("SPARSE_QP_SOLUTION",        "CONDENSING"),
                    ("QP_SOLVER",                 "QP_QPOASES"),
#                    ("SPARSE_QP_SOLUTION",        "SPARSE_SOLVER"),
#                    ("QP_SOLVER",                 "QP_FORCES"),
                    ("FIX_INITIAL_STATE",         "YES"),
                    ("HOTSTART_QP",               "NO"),
                    ("GENERATE_MATLAB_INTERFACE", "YES")]
    mpcRt = mpc.exportCode(cgOptions=cgOptions,
                           acadoOptions=acadoOptions,
                           qpSolver='QP_OASES')
    return mpcRt


def makeMhe(dae, N, ts):
    mhe = rawe.ocp.Ocp(dae, N=N, ts=ts)

    mhe.minimizeLsq([mhe['pos'],mhe['vel']])
    mhe.minimizeLsqEndTerm([mhe['pos'],mhe['vel']])

#    cgOptions = {'CXX':'clang++', 'CC':'clang'}
    cgOptions = {'CXX':'g++', 'CC':'gcc'}
#    cgOptions = {'CXX':'icpc', 'CC':'icc'}
    acadoOptions = [("HESSIAN_APPROXIMATION",     "GAUSS_NEWTON"),
                    ("DISCRETIZATION_TYPE",       "MULTIPLE_SHOOTING"),
                    ("INTEGRATOR_TYPE",           "INT_IRK_RIIA3"),
                    ("NUM_INTEGRATOR_STEPS",      str(N*5)),
                    ("LINEAR_ALGEBRA_SOLVER",     "GAUSS_LU"),
                    ("SPARSE_QP_SOLUTION",        "FULL_CONDENSING"),
#                    ("SPARSE_QP_SOLUTION",        "CONDENSING"),
                    ("QP_SOLVER",                 "QP_QPOASES"),
#                    ("SPARSE_QP_SOLUTION",        "SPARSE_SOLVER"),
#                    ("QP_SOLVER",                 "QP_FORCES"),
                    ("FIX_INITIAL_STATE",         "NO"),
                    ("HOTSTART_QP",               "NO"),
                    ("GENERATE_MATLAB_INTERFACE", "YES")]
    mheRt = mhe.exportCode(cgOptions=cgOptions,
                           acadoOptions=acadoOptions,
                           qpSolver='QP_OASES')
    return mheRt


if __name__=='__main__':
    N = 50
    ts = 0.2

    A = 0.5
    omega = 0.3

    dae = makeDae()
    mpc = makeMpc(dae, N, ts)

#    sim = rawe.dae.rienIntegrator.RienIntegrator(dae, ts=ts)
    sim = rawe.sim.Sim(dae, ts=ts)

    print '='*80

    # set the mpc weights
    xRms = 0.1
    vRms = omega*xRms
    fRms = 0.3
    mpc.S[0,0] = (1.0/xRms)**2
    mpc.S[1,1] = (1.0/vRms)**2
    mpc.S[2,2] = (1.0/fRms)**2
    mpc.SN[0,0] = (1.0/xRms)**2
    mpc.SN[1,1] = (1.0/vRms)**2

    # initial guess
    tk = 0
    for k in range(N+1):
        mpc.x[k,0] = A*C.sin(omega*tk)
        mpc.x[k,1] = A*C.cos(omega*tk)*omega
        tk += ts

    # plot initial guess
    plt.figure()
    plt.subplot(311)
    plt.plot(mpc.x[:,0])
    plt.title('x initial guess')
    plt.subplot(312)
    plt.plot(mpc.x[:,1])
    plt.title('v initial guess')
    plt.subplot(313)
    plt.plot(mpc.u[:,0])
    plt.title('u initial guess')

    # set tracking trajectory
    t = 0
    def setTrajectory(ocp, t0):
        tk = t0
        for k in range(N):
            ocp.y[k,0] = A*C.sin(omega*tk)
            ocp.y[k,1] = A*C.cos(omega*tk)*omega
            tk += ts
        ocp.yN[0] = A*C.sin(omega*tk)
        ocp.yN[1] = A*C.cos(omega*tk)*omega
    setTrajectory(mpc, t)

    # run a sim
    x = {'pos':0,'vel':mpc.x[k,1]}
    u = {'force':0}

    ytraj = []
    xtraj = []
    xestTraj = []
    utraj = []
    kkts = []
    objs = []
    t = 0
    for k in range(200):
        mpc.x0[0] = x['pos'] # will be mhe output
        mpc.x0[1] = x['vel'] # will be mhe output

        # command mpc to follow sin wave
        setTrajectory(mpc, t)
        
        # log stuff
        xtraj.append((x['pos'], x['vel']))
        ytraj.append((mpc.y[0,0], mpc.y[0,1]))
        xestTraj.append((mpc.x0[0], mpc.x0[1]))

        # run mpc
        mpc.preparationStep()
        fbret = mpc.feedbackStep()
        if fbret != 0:
            raise Exception("feedbackStep returned error code "+str(fbret))

        for j in range(10):
            mpc.preparationStep()
            fbret = mpc.feedbackStep()
            if fbret != 0:
                raise Exception("feedbackStep returned error code "+str(fbret))
        u['force'] = mpc.u[0,0]

        # log
        utraj.append(u['force'])
        kkts.append(mpc.getKKT())
        objs.append(mpc.getObjective())

        # simulate
        x = sim.step(x, u, {})
        t += ts
        
    # plot results
    plt.figure()
    plt.subplot(311)
    plt.plot([xt[0] for xt in ytraj])
    plt.plot([xt[0] for xt in xtraj])
    plt.legend(['command','actual'])
    plt.title('x')

    plt.subplot(312)
    plt.plot([xt[1] for xt in ytraj])
    plt.plot([xt[1] for xt in xtraj])
    plt.legend(['command','actual'])
    plt.title('v')

    plt.subplot(313)
    plt.plot(utraj)
    plt.title('u')

    plt.figure()
    plt.subplot(211)
    plt.semilogy(kkts)
    plt.title('kkts')
    plt.subplot(212)
    plt.semilogy(objs)
    plt.title('objectives')

    plt.show()