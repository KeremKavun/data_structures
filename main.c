#include <stdio.h>
#include "hash_table.h"

int main() {
    // Create a new hash table
    ht_hash_table* ht = ht_new();

    // Insert some key-value pairs
    ht_insert(ht, "name", "Alice");
    ht_insert(ht, "age", "30");
    ht_insert(ht, "city", "New York");

    // Retrieve and print values
    char* name = ht_search(ht, "name");
    char* age = ht_search(ht, "age");
    char* city = ht_search(ht, "city");

    printf("Name: %s\n", name);
    printf("Age: %s\n", age);
    printf("City: %s\n", city);

    // Delete a key-value pair
    printf("Deleting age...\n");
    ht_delete(ht, "age");

    // Try to retrieve the deleted value
    printf("Retrieving deleted age...\n");
    age = ht_search(ht, "age");
    printf("Deleted Age: %s\n", age ? age : "Not found");

    // Clean up
    printf("Cleaning up hash table...\n");
    ht_del_hash_table(ht);

    return 0;
}