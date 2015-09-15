SimWare v1.0.1

=== Latest Release ===
http://arch.ece.gatech.edu/research/research-simware.html

=== Project Description ===
This project represents a holistic warehouse-scale computer simulator SimWare. Using this tool, users can perform a comprehensive analysis for the energy consumption of data centers using real utilization traces. The motivation of this work comes from the fact that increasing the room temperature of a data center does not always guarantee better energy efficiency due to the ignorance of several factors such as increased fan power inside servers. To evaluate energy saving policy more accurately, one must consider the cooling units and computing components holistically as we have implemented in SimWare to include the following critical components:
- The power consumption of servers as a function of workload, inlet air temperature, and fan power.
- The power consumption of cooling units as a function of supply air temperature.
- The heat flow and recirculation effect to keep every server operate at the different inlet air temperature.
Using SimWare, one will understand better the causes of energy inefficiency in a data center under different power management schemes, and contrive more effective solutions to minimize them. We release SimWare to the research community to promote our holistic data center simulation methodology as well as to enable more research activities for green data center design.

=== How to compile ===
SimWare uses Boost (http://www.boost.org/) library. For compiling on linux, please edit the second line of SimWare/Makefile to include boost library. Current setting uses ~/boost. For compiling on Windows using Visual Studio, please edit SimWare/SimWare.vcxproj to include Boost library. Current setting uses C:\Program Files (x86)\boost\boost_1_47; and C:\Program Files (x86)\boost\boost_1_47\lib;

