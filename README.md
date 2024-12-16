This project proposes the implementation of the research paper "Fabrice Theoleyre, A route-aware MAC for wireless multihop networks with a convergecast traffic pattern , Computer Networks, 2011, 55(3), pp 822-837, https://doi.org/10.1016/j.comnet.2010.10.018

We propose first to select a k-tree core, i.e. a sub-tree of the shortest paths to the sink containing exactly k-leaves. In particular, these k-tree core nodes are chosen among the nodes that must forward most traffic. We design c-mac, an optimized MAC layer for this kind of topology. c-mac is derived from the CSMA-CA like approaches and consists in giving a larger priority to the k-tree core nodes. Moreover, a proper coordination among the k-tree core nodes permits to limit collisions among them. Simulation results show that organizing the transmissions in c-mac permits to achieve a much larger throughput than the original ieee 802.11 – like protocol it is based on. This simple solution can be adapted to most CSMA-CA like protocols, and is particularly relevant for WSN or WMN in which traffic is mostly destined to the sink/gateway.
