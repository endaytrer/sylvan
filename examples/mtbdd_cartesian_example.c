#include <stdio.h>
#include <stdint.h>
#include <sylvan.h>
#include <sylvan_mtbdd.h>
#include <sylvan_mt.h>
#include <lace.h>





// Function to evaluate an integer MTBDD for a given variable assignment
// mtbdd: the MTBDD to evaluate
// assignment: array of boolean values (0=false, 1=true) for variables 0, 1, 2, ...
// max_var: the maximum variable index in the MTBDD
int32_t evaluate_mtbdd_int(MTBDD mtbdd, uint8_t *assignment, uint32_t max_var) {
    // If it's a leaf node, return its value
    if (!mtbdd_isnode(mtbdd)) {
        return (int32_t)mtbdd_getint64(mtbdd);
    }
    
    // Get the current variable index
    uint32_t var = mtbdd_getvar(mtbdd);
    
    // If variable index exceeds max_var, return 0 (or handle as needed)
    if (var > max_var) {
        return 0;
    }
    
    // Choose the branch based on the assignment
    if (assignment[var] == 0) {
        // Take the low branch (false)
        return evaluate_mtbdd_int(mtbdd_getlow(mtbdd), assignment, max_var);
    } else {
        // Take the high branch (true)
        return evaluate_mtbdd_int(mtbdd_gethigh(mtbdd), assignment, max_var);
    }
}

// Function to evaluate a pair MTBDD for a given variable assignment
// mtbdd: the MTBDD to evaluate (result of Cartesian product)
// assignment: array of boolean values (0=false, 1=true) for variables 0, 1, 2, ...
// max_var: the maximum variable index in the MTBDD
void evaluate_mtbdd_pair(MTBDD mtbdd, uint8_t *assignment, uint32_t max_var, int32_t *first, int32_t *second) {
    // If it's a leaf node, return its value
    if (!mtbdd_isnode(mtbdd)) {
        *first = mtbdd_getint32_pair_first(mtbdd);
        *second = mtbdd_getint32_pair_second(mtbdd);
        return;
    }
    
    // Get the current variable index
    uint32_t var = mtbdd_getvar(mtbdd);
    
    // If variable index exceeds max_var, return (0, 0) (or handle as needed)
    if (var > max_var) {
        *first = 0;
        *second = 0;
        return;
    }
    
    // Choose the branch based on the assignment
    if (assignment[var] == 0) {
        // Take the low branch (false)
        evaluate_mtbdd_pair(mtbdd_getlow(mtbdd), assignment, max_var, first, second);
    } else {
        // Take the high branch (true)
        evaluate_mtbdd_pair(mtbdd_gethigh(mtbdd), assignment, max_var, first, second);
    }
}

// Main task function to be run by Lace
VOID_TASK_0(main_task)
{
    // Initialize Sylvan
    printf("Initializing Sylvan...\n");
    sylvan_set_sizes(1LL<<22, 1LL<<26, 1LL<<22, 1LL<<26);
    sylvan_init_package();
    sylvan_init_mtbdd();
    
    printf("Sylvan MTBDD Cartesian Product Example\n");
    
    // Create integer leaves
    MTBDD leaf2 = mtbdd_int64(2);
    MTBDD leaf4 = mtbdd_int64(4);
    MTBDD leaf6 = mtbdd_int64(6);
    MTBDD leaf8 = mtbdd_int64(8);
    MTBDD leaf10 = mtbdd_int64(10);
    
    printf("Created integer leaves: 2, 4, 6, 8, 10\n");
    
    // Create first MTBDD: A(var0) = if var0 then 6 else 2
    MTBDD mtbdd_a = mtbdd_makenode(0, leaf2, leaf6);
    
    // Create second MTBDD: B(var0, var1) = if var0 then (if var1 then 10 else 8) else 4
    MTBDD mtbdd_b_inner = mtbdd_makenode(1, leaf8, leaf10);
    MTBDD mtbdd_b = mtbdd_makenode(0, leaf4, mtbdd_b_inner);
    
    printf("Created MTBDD A: A(var0) = if var0 then 6 else 2\n");
    printf("Created MTBDD B: B(var0, var1) = if var0 then (if var1 then 10 else 8) else 4\n");
    
    // Apply Cartesian product operation to get C(var0, var1) = A × B
    MTBDD mtbdd_cartesian_product = mtbdd_cartesian(mtbdd_a, mtbdd_b);
    
    printf("Computed Cartesian product C = A × B\n");
    
    // Evaluate the MTBDDs for various input assignments
    // Assignment format: [var0, var1] where 0=false, 1=true
    uint32_t max_var = 1; // Maximum variable index in our MTBDDs
    
    printf("\nEvaluating MTBDDs for various inputs:\n");
    printf("Input (var0, var1) | A(var0) | B(var0, var1) | C(var0, var1)\n");
    printf("-------------------|---------|--------------|----------------\n");
    
    // Evaluate for (0, 0)
    uint8_t assign00[] = {0, 0};
    int32_t a00 = evaluate_mtbdd_int(mtbdd_a, assign00, max_var);
    int32_t b00 = evaluate_mtbdd_int(mtbdd_b, assign00, max_var);
    int32_t c00_first, c00_second;
    evaluate_mtbdd_pair(mtbdd_cartesian_product, assign00, max_var, &c00_first, &c00_second);
    printf("(0, 0)             | %-7d | %-12d | (%d, %d)\n", a00, b00, c00_first, c00_second);
    
    // Evaluate for (0, 1)
    uint8_t assign01[] = {0, 1};
    int32_t a01 = evaluate_mtbdd_int(mtbdd_a, assign01, max_var);
    int32_t b01 = evaluate_mtbdd_int(mtbdd_b, assign01, max_var);
    int32_t c01_first, c01_second;
    evaluate_mtbdd_pair(mtbdd_cartesian_product, assign01, max_var, &c01_first, &c01_second);
    printf("(0, 1)             | %-7d | %-12d | (%d, %d)\n", a01, b01, c01_first, c01_second);
    
    // Evaluate for (1, 0)
    uint8_t assign10[] = {1, 0};
    int32_t a10 = evaluate_mtbdd_int(mtbdd_a, assign10, max_var);
    int32_t b10 = evaluate_mtbdd_int(mtbdd_b, assign10, max_var);
    int32_t c10_first, c10_second;
    evaluate_mtbdd_pair(mtbdd_cartesian_product, assign10, max_var, &c10_first, &c10_second);
    printf("(1, 0)             | %-7d | %-12d | (%d, %d)\n", a10, b10, c10_first, c10_second);
    
    // Evaluate for (1, 1)
    uint8_t assign11[] = {1, 1};
    int32_t a11 = evaluate_mtbdd_int(mtbdd_a, assign11, max_var);
    int32_t b11 = evaluate_mtbdd_int(mtbdd_b, assign11, max_var);
    int32_t c11_first, c11_second;
    evaluate_mtbdd_pair(mtbdd_cartesian_product, assign11, max_var, &c11_first, &c11_second);
    printf("(1, 1)             | %-7d | %-12d | (%d, %d)\n", a11, b11, c11_first, c11_second);
    
    // Clean up
    sylvan_quit();
    
    printf("\nDone\n");
}

int main() {
    int n_workers = 0; // Use 1 worker for simplicity
    size_t deque_size = 0; // Default deque size
    
    // Initialize the Lace framework
    lace_start(n_workers, deque_size);
    
    // Run the main task
    RUN(main_task);
    
    // Stop the Lace framework
    lace_stop();
    
    return 0;
}
