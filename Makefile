build:
	g++ --std=c++17 -g -O0 OrderCacheTest.cpp OrderCache.cpp -o OrderCacheTest -lgtest -lgtest_main -pthread -I"${CONDA_PREFIX}"/include -L"${CONDA_PREFIX}"/lib

clean:
	rm OrderCacheTest