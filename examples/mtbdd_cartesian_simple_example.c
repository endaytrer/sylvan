#include <stdio.h>
#include <stdint.h>
#include <sylvan.h>
#include <sylvan_mtbdd.h>
#include <lace.h>

// Function to evaluate an MTBDD with 32-bit integer leaves
int32_t evaluate_mtbdd_int(MTBDD mtbdd, uint8_t *assignment, uint32_t max_var) {
    // If it's a leaf node, return its value
    if (!mtbdd_isnode(mtbdd)) {
        return (int32_t)mtbdd_getint64(mtbdd);
    }
    
    // Get the current variable index
    uint32_t var = mtbdd_getvar(mtbdd);
    
    // If variable index exceeds max_var, return 0
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

// Function to evaluate an MTBDD with 32-bit integer pair leaves
void evaluate_mtbdd_pair(MTBDD mtbdd, uint8_t *assignment, uint32_t max_var, int32_t *first, int32_t *second) {
    // If it's a leaf node, return its value as a pair
    if (!mtbdd_isnode(mtbdd)) {
        *first = mtbdd_getint32_pair_first(mtbdd);
        *second = mtbdd_getint32_pair_second(mtbdd);
        return;
    }
    
    // Get the current variable index
    uint32_t var = mtbdd_getvar(mtbdd);
    
    // If variable index exceeds max_var, return default pair (0, 0)
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
VOID_TASK_0(main_task) {
    // Initialize Sylvan
    printf("Initializing Sylvan...\n");
    
    sylvan_set_sizes(1LL<<22, 1LL<<26, 1LL<<22, 1LL<<26);
    sylvan_init_package();
    sylvan_init_mtbdd();
    
    printf("Sylvan MTBDD Simple Cartesian Product Example\n");
    
    // Create simple MTBDD A: f(var0) = if var0 then 5 else 3
    MTBDD leaf3 = mtbdd_int64(3);
    MTBDD leaf5 = mtbdd_int64(5);
    MTBDD mtbdd_A = mtbdd_makenode(0, leaf3, leaf5);
    
    printf("Created MTBDD A: var0 ? 5 : 3\n");
    
    // Create simple MTBDD B: g(var0, var1) = if var0 then (var1 ? 7 : 4) else 2
    MTBDD leaf2 = mtbdd_int64(2);
    MTBDD leaf4 = mtbdd_int64(4);
    MTBDD leaf7 = mtbdd_int64(7);
    MTBDD inner_node = mtbdd_makenode(1, leaf4, leaf7);
    MTBDD mtbdd_B = mtbdd_makenode(0, leaf2, inner_node);
    
    printf("Created MTBDD B: var0 ? (var1 ? 7 : 4) : 2\n");
    
    // Evaluate individual MTBDDs for verification
    uint32_t max_var = 1; // Maximum variable index in our MTBDDs
    uint8_t assignments[4][2] = { {0, 0}, {0, 1}, {1, 0}, {1, 1} };
    char *assign_strs[4] = { "(0, 0)", "(0, 1)", "(1, 0)", "(1, 1)" };
    
    printf("\nEvaluating individual MTBDDs:\n");
    for (int i = 0; i < 4; i++) {
        int32_t val_A = evaluate_mtbdd_int(mtbdd_A, assignments[i], max_var);
        int32_t val_B = evaluate_mtbdd_int(mtbdd_B, assignments[i], max_var);
        printf("A%s = %d, B%s = %d\n", assign_strs[i], val_A, assign_strs[i], val_B);
    }
    
    // Apply built-in Cartesian product operation on the two MTBDDs
    MTBDD cartesian_product = mtbdd_cartesian(mtbdd_A, mtbdd_B);
    
    printf("\nApplied Cartesian product operation to create A×B\n");
    
    // Evaluate the Cartesian product MTBDD for the same input assignments
    printf("\nEvaluating Cartesian product MTBDD A×B:\n");
    for (int i = 0; i < 4; i++) {
        int32_t first, second;
        evaluate_mtbdd_pair(cartesian_product, assignments[i], max_var, &first, &second);
        printf("(A×B)%s = (%d, %d)\n", assign_strs[i], first, second);
    }
    
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
