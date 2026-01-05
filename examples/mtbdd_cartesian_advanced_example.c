#include <stdio.h>
#include <stdint.h>
#include <sylvan.h>
#include <sylvan_mtbdd.h>
#include <lace.h>

// Function to evaluate an integer MTBDD for a given variable assignment
int64_t evaluate_mtbdd_int(MTBDD mtbdd, uint8_t *assignment, uint32_t max_var) {
    if (!mtbdd_isnode(mtbdd)) {
        return mtbdd_getint64(mtbdd);
    }
    
    uint32_t var = mtbdd_getvar(mtbdd);
    if (var > max_var) {
        return 0;
    }
    
    if (assignment[var] == 0) {
        return evaluate_mtbdd_int(mtbdd_getlow(mtbdd), assignment, max_var);
    } else {
        return evaluate_mtbdd_int(mtbdd_gethigh(mtbdd), assignment, max_var);
    }
}

// Function to evaluate a pair MTBDD for a given variable assignment
void evaluate_mtbdd_pair(MTBDD mtbdd, uint8_t *assignment, uint32_t max_var, int32_t *first, int32_t *second) {
    if (!mtbdd_isnode(mtbdd)) {
        *first = mtbdd_getint32_pair_first(mtbdd);
        *second = mtbdd_getint32_pair_second(mtbdd);
        return;
    }
    
    uint32_t var = mtbdd_getvar(mtbdd);
    if (var > max_var) {
        *first = 0;
        *second = 0;
        return;
    }
    
    if (assignment[var] == 0) {
        evaluate_mtbdd_pair(mtbdd_getlow(mtbdd), assignment, max_var, first, second);
    } else {
        evaluate_mtbdd_pair(mtbdd_gethigh(mtbdd), assignment, max_var, first, second);
    }
}

// Advanced example: Compute Cartesian product of two more complex functions
// f(x, y) = 2x + y
// g(x, y) = x - y
// Then compute f × g and evaluate it for various (x, y) inputs

// Main task function to be run by Lace
VOID_TASK_0(main_task)
{
    // Initialize Sylvan
    printf("Initializing Sylvan...\n");
    sylvan_set_sizes(1LL<<22, 1LL<<26, 1LL<<22, 1LL<<26);
    sylvan_init_package();
    sylvan_init_mtbdd();
    
    printf("\nSylvan MTBDD Advanced Cartesian Product Example\n");
    printf("==============================================\n");
    
    // This example demonstrates the Cartesian product of two more complex MTBDDs:
    // f(x, y) = 2x + y, where x is var0 and y is var1
    // g(x, y) = x - y, where x is var0 and y is var1
    
    // Create integer leaves for the base values
    MTBDD leaf0 = mtbdd_int64(0);
    MTBDD leaf1 = mtbdd_int64(1);
    MTBDD leaf2 = mtbdd_int64(2);
    MTBDD leaf3 = mtbdd_int64(3);
    MTBDD leaf_1 = mtbdd_int64(-1);
    
    printf("Created integer leaves: 0, 1, 2, 3, -1\n");
    
    // Build function f(x, y) = 2x + y
    printf("\nBuilding f(x, y) = 2x + y (x=var0, y=var1):\n");
    
    // For x=0 (var0=0), f(0, y) = 0 + y:
    //   y=0 → 0
    //   y=1 → 1
    MTBDD f_x0 = mtbdd_makenode(1, leaf0, leaf1);
    
    // For x=1 (var0=1), f(1, y) = 2 + y:
    //   y=0 → 2
    //   y=1 → 3
    MTBDD f_x1 = mtbdd_makenode(1, leaf2, leaf3);
    
    // Root node for f: if var0 then f_x1 else f_x0
    MTBDD f = mtbdd_makenode(0, f_x0, f_x1);
    
    printf("  f(0, 0) = 0\n");
    printf("  f(0, 1) = 1\n");
    printf("  f(1, 0) = 2\n");
    printf("  f(1, 1) = 3\n");
    
    // Build function g(x, y) = x - y
    printf("\nBuilding g(x, y) = x - y (x=var0, y=var1):\n");
    
    // For x=0 (var0=0), g(0, y) = 0 - y:
    //   y=0 → 0
    //   y=1 → -1
    MTBDD g_x0 = mtbdd_makenode(1, leaf0, leaf_1);
    
    // For x=1 (var0=1), g(1, y) = 1 - y:
    //   y=0 → 1
    //   y=1 → 0
    MTBDD g_x1 = mtbdd_makenode(1, leaf1, leaf0);
    
    // Root node for g: if var0 then g_x1 else g_x0
    MTBDD g = mtbdd_makenode(0, g_x0, g_x1);
    
    printf("  g(0, 0) = 0\n");
    printf("  g(0, 1) = -1\n");
    printf("  g(1, 0) = 1\n");
    printf("  g(1, 1) = 0\n");
    
    // Compute Cartesian product f × g using the built-in function
    printf("\nComputing Cartesian product: h(x, y) = (f(x, y), g(x, y))\n");
    MTBDD h = mtbdd_cartesian(f, g);
    
    // Evaluate all possible inputs (x, y) where x and y can be 0 or 1
    uint32_t max_var = 1; // Maximum variable index in our MTBDDs
    uint8_t assignments[4][2] = {
        {0, 0},  // (x=0, y=0)
        {0, 1},  // (x=0, y=1)
        {1, 0},  // (x=1, y=0)
        {1, 1}   // (x=1, y=1)
    };
    
    printf("\nEvaluating all possible inputs:\n");
    printf("Input (x, y) | f(x, y) | g(x, y) | h(x, y) = (f, g)\n");
    printf("-------------|---------|---------|---------------\n");
    
    for (int i = 0; i < 4; i++) {
        uint8_t *assign = assignments[i];
        int x = assign[0];
        int y = assign[1];
        
        // Evaluate individual functions
        int64_t f_val = evaluate_mtbdd_int(f, assign, max_var);
        int64_t g_val = evaluate_mtbdd_int(g, assign, max_var);
        
        // Evaluate Cartesian product
        int32_t first, second;
        evaluate_mtbdd_pair(h, assign, max_var, &first, &second);
        
        printf("(%d, %d)       | %-7lld | %-7lld | (%d, %d)\n", 
               x, y, f_val, g_val, first, second);
    }
    
    // Now let's demonstrate something more advanced:
    // Compute the product of the pair elements for each input
    printf("\nAdvanced Operation: Compute product of pair elements (f × g)\n");
    printf("Input (x, y) | h(x, y) = (f, g) | f × g\n");
    printf("-------------|---------------|-------\n");
    
    for (int i = 0; i < 4; i++) {
        uint8_t *assign = assignments[i];
        int x = assign[0];
        int y = assign[1];
        
        // Evaluate Cartesian product and compute the product
        int32_t first, second;
        evaluate_mtbdd_pair(h, assign, max_var, &first, &second);
        int64_t product = (int64_t)first * (int64_t)second;
        
        printf("(%d, %d)       | (%d, %d)     | %lld\n", 
               x, y, first, second, product);
    }
    
    printf("\nThis demonstrates how the Cartesian product can be used to:");
    printf("\n1. Combine multiple functions into a single MTBDD");
    printf("\n2. Evaluate all functions simultaneously with a single input");
    printf("\n3. Perform further operations on the combined results\n");
    
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