all:
	g++ -std=c++17 -O3 -flto -Wall -Wextra Sim.cpp Sim_Math.cpp TimeCode.cpp BigInteger.cpp -o traffic-simulation -lsqlite3 -I/usr/include

tests:
	g++ -std=c++17 -O3 -flto -Wall -Wextra Tests.cpp Sim_Math.cpp TimeCode.cpp TimeCodeTests.cpp BigInteger.cpp BigIntegerTests.cpp -o tests -lsqlite3 -I/usr/include

clean:
	rm -rf tests traffic-simulation *.o plot.html