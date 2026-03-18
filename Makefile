CXX = g++
CXXFLAGS = -O3 -mavx2 -Wall

SRC_DIR = cpp_implementation
OBJ_DIR = obj

CORE_SRC = $(SRC_DIR)/Weyl.cpp
CORE_OBJ = $(SRC_DIR)/$(OBJ_DIR)/Weyl.o

MAIN_BENCHMARK = $(SRC_DIR)/TestWeylPerformance.cpp
MAIN_PATCH = $(SRC_DIR)/TestPatchMatching.cpp
MAIN_CORRESP = $(SRC_DIR)/TestDenseCorresponding.cpp
MAIN_METRICS = $(SRC_DIR)/TestMetrics.cpp

EXEC_BENCHMARK = weyl_benchmark
EXEC_PATCH = weyl_patch_matching
EXEC_CORRESP = weyl_dense_corresponding
EXEC_METRICS = weyl_metrics


all: benchmark patch_matching dense_corresponding metrics_evaluation

$(CORE_OBJ): $(CORE_SRC) $(SRC_DIR)/Weyl.hpp
	mkdir -p $(SRC_DIR)/$(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $(CORE_SRC) -o $(CORE_OBJ)


benchmark: $(CORE_OBJ) $(MAIN_BENCHMARK)
	$(CXX) $(CXXFLAGS) $(MAIN_BENCHMARK) $(CORE_OBJ) -o $(EXEC_BENCHMARK)
	@echo "[SUCCÈS] Exécutable du benchmark généré : ./"$(EXEC_BENCHMARK)

patch_matching: $(CORE_OBJ) $(MAIN_PATCH)
	$(CXX) $(CXXFLAGS) $(MAIN_PATCH) $(CORE_OBJ) -o $(EXEC_PATCH)
	@echo "[SUCCÈS] Exécutable du patch matching généré : ./"$(EXEC_PATCH)

dense_corresponding: $(CORE_OBJ) $(MAIN_CORRESP)
	$(CXX) $(CXXFLAGS) $(MAIN_CORRESP) $(CORE_OBJ) -o $(EXEC_CORRESP)
	@echo "[SUCCÈS] Exécutable du dense corresponding généré : ./"$(EXEC_CORRESP)

metrics_evaluation: $(CORE_OBJ) $(MAIN_METRICS)
	$(CXX) $(CXXFLAGS) $(MAIN_METRICS) $(CORE_OBJ) -o $(EXEC_METRICS)
	@echo "[SUCCÈS] Exécutable de l'évaluation de metriques généré : ./"$(EXEC_METRICS)

clean:
	rm -f $(EXEC_BENCHMARK) $(EXEC_PATCH)
	rm -rf $(SRC_DIR)/$(OBJ_DIR)
	@echo "[INFO] Nettoyage terminé."