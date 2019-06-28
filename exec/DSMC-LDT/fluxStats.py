import numpy as np
import matplotlib.pyplot as plt
import sys

def getData(data):
  #extract data from a flux file

  #declare lists
  lFlux = []
  rFlux = []
  netFlux = []

  #counter for first few arguments
  counter = 0

  #open file with data
  file = open(data,"r")

  #Read in each line
  for line in file:
    
    entries = line.split()

    if counter == 0:
      dt = float(entries[0])
    elif counter == 1:
      lN = int(entries[0])
      rN = int(entries[1])
    elif counter == 2:
      lT = int(entries[0])
      rT = int(entries[1])
    else:
      lFlux.append(int(entries[0]))
      rFlux.append(int(entries[1]))

    counter = counter + 1
    
  #close file   
  file.close()

  #get the net flux - particles going right - left
  N = len(lFlux)
  for i in xrange(0,N):
    netFlux.append((rFlux[i]) - (lFlux[i]))

  return dt, lN, rN, lT, rT, lFlux, rFlux, netFlux

def makeHist(flux):
  #make a histogram with the data in flux

  #make some histograms
  hist, bin_edges = np.histogram(flux)

  #plot some histograms
  # An "interface" to matplotlib.axes.Axes.hist() method
  n, bins, patches = plt.hist(x=flux, bins='auto', color='#0504aa',
                              alpha=0.7, rwidth=0.85)
  plt.grid(axis='y', alpha=0.75)
  plt.xlabel('Value')
  plt.ylabel('Frequency')
  plt.title('Flux Histogram')
  maxfreq = n.max()
  # Set a clean upper y-axis limit.
  plt.ylim(ymax=np.ceil(maxfreq / 10) * 10 if maxfreq % 10 else maxfreq + 10)
  plt.show()

  return

def getAverageFlux(N, ts):
  #plot the average flux as a function of timestep

  #construct storage for data - first element is 0
  cumFlux = [0.0 for _ in range(ts+1)]

  #base for data location
  base = "samples/fluxes"

  #loop over all the data
  for i in xrange(0,N):
    #get the data
    loc = base + str(i) + ".txt"
    dt, lN, rN, lT, rT, lFlux, rFlux, netFlux = getData(loc)

    #compute a cumulative sum of netFlux
    cs = np.cumsum(netFlux)
    cs = np.insert(cs,0,0) #append a zero as the first element
    
    #add the cumsum to cumFlux
    cumFlux = np.add(cumFlux, cs)

  #divide cumFlux by number of samples
  Ninv = 1.0 / N
  #cumFlux.astype(float)
  cumFlux[:] = [x * Ninv for x in cumFlux]

  return cumFlux

def plotAverageFlux(avgFlux, N, ts, dt, eBarFlag, theory):
  #plot the average flux

  #get the theory curve
  t = xrange(0,ts+1)
  Th = np.ones(ts+1)*theory

  if (eBarFlag == 1):
    eBars = []

    #get error bars every 500th time step
    for i in xrange(0,ts+1):
      if (i % 500 == 0):
        eBars.append(getErrorBar(N, i))
        print "Got error bar at ", i
      else:
        eBars.append(0)

    #plot the data
    plt.errorbar(t, avgFlux, eBars, errorevery = 500)
  else:
    #plot the data
    plt.plot(t, avgFlux)

  plt.plot(t,Th)
  plt.title('Average Flux over time')
  plt.xlabel('Time Step')
  plt.ylabel('Flux')
  plt.legend(['DSMC', 'Theory Average'])
  plt.show()

  return

def getErrorBar(N, time):
  #get error bar on average flux at time step = time

  #construct storage for data - first element is 0
  cumFlux = [0.0 for _ in range(ts+1)]

  #base for data location
  base = "samples/fluxes"

  #hold all cumFLuxes at given time step
  measurements = []

  #loop over all the data
  for i in xrange(0,N):
    #get the data
    loc = base + str(i) + ".txt"
    dt, lN, rN, lT, rT, lFlux, rFlux, netFlux = getData(loc)

    #compute cumsum
    cs = np.cumsum(netFlux)
    cs = np.insert(cs,0,0) #append a zero as the first element

    measurements.append(cs[time]);

  eBar = np.std(measurements)

  return eBar

def estimateRates(lFlux, rFlux, netFlux, lN, rN, dt):
  #estimate a rate of crossing using data
  #A->B is rFlux, B->A is lFLux. N is original amount

  #create number of particles in each reservoir as fn of time
  lcount = lN
  rcount = rN
  lP = []
  rP = []
  lP.append(lcount)
  rP.append(rcount)
  ts = len(netFlux)
  for i in xrange(0,ts):
    lcount = lcount - netFlux[i]
    rcount = rcount + netFlux[i]
    lP.append(lcount)
    rP.append(rcount)

  #get rates
  lrate = []
  rrate = []
  ltime = 0
  rtime = 0
  for i in xrange(1000,ts):
    ltime = ltime + 1
    rtime = rtime + 1
    if rFlux[i] > 0:
      #print rP[i-1], rP[i], rP[i+1]
      rrate.append((float(rFlux[i])/lP[i])/(rtime*dt))
      rtime = 0
    if lFlux[i] > 0:
      lrate.append((float(lFlux[i])/rP[i])/(ltime*dt))
      ltime = 0

  return lrate, rrate

def estimateRatesAll(N):
  #estimate the rates using every trial

  lrates = []
  rrates = []

  #base for data location
  base = "samples/fluxes"

  #loop over all the data
  for i in xrange(0,N):
    #get the data
    loc = base + str(i) + ".txt"
    dt, lN, rN, lT, rT, lFlux, rFlux, netFlux = getData(loc)

    lrate, rrate = estimateRates(lFlux, rFlux, netFlux, lN, rN, dt)
    lsample = np.mean(lrate)
    rsample = np.mean(rrate)

    lrates.append(lsample)
    rrates.append(rsample)

  return lrates, rrates










if __name__ == "__main__":

    if len(sys.argv) != 4:
        print("Usage: python fluxStats.py <numSamples> <numTimeSteps> <plotEbar>")

    #store inputs
    N = int(sys.argv[1])
    ts = int(sys.argv[2])
    eBarFlag = int(sys.argv[3])

    #get an example run for parameters
    dt, lN, rN, lT, rT, lFlux, rFlux, netFlux = getData("samples/fluxes5.txt")
    print "Temperatures: ", lT, rT

    #estimate rates
    #lrate, rrate = estimateRates(lFlux, rFlux, netFlux, lN, rN, dt)

    lrates, rrates = estimateRatesAll(N)
    #makeHist(lrates)
    #makeHist(rrates)
    lm = np.mean(lrates)
    rm = np.mean(rrates)
    print lm, rm

    #compute expected flux from theory using temperatures
    theory = np.sqrt(lT)/(np.sqrt(lT)+np.sqrt(rT)) * (lN+rN) - rN
    #theory = np.power(float(lT),0.95)/(np.power(lT,0.95)+np.power(rT,0.95)) * (lN+rN) - rN
    #theory = rm/(lm+rm) * (lN+rN) - rN
    print theory

    #get the average flux as a function of timestep
    print "Getting Average Flux..."
    avgFlux = getAverageFlux(N, ts)
    plotAverageFlux(avgFlux, N, ts, dt, eBarFlag, theory)

    #estimate rates
    #lrate, rrate = estimateRates(lFlux, rFlux, netFlux, lN, rN, dt)

    #lrates, rrates = estimateRatesAll(N)
    #makeHist(lrates)
    #makeHist(rrates)









