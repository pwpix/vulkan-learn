
#include <iostream>

// --- Step 1: Create our custom Deleters ---
// These are simple structs that know how to clean up specific resources.
struct IntPointerDeleter {
    void operator() (int* ptr) const
    {
        std::cout << "-> Deleter: Freeing int memory!\n";
        delete ptr;
    }
};

struct FileDeleter {
    void operator() (const char* filename) const
    {
        std::cout << "-> Deleter: Closing file: " << filename << "!\n";
        // In a real app, you would call CloseHandle() or fclose() here
    }
};


// --- Step 2: The Templated Wrapper (Your Task!) ---
template <typename T, typename Deleter>
struct UniqueHandle {
    T resource;

    // Constructor initializes the resource
    UniqueHandle (T res) : resource (res) {}

    // 1. TODO: Write the Destructor (~UniqueHandle)
    // It must call the Deleter on the resource.
    // Hint: Instantiate the Deleter struct like a function: Deleter{}(resource);
    ~UniqueHandle ()
    {
        Deleter{}(resource);
    }


    // 2. TODO: Prevent copying!
    // Because this wrapper owns the resource exclusively, copying it
    // would cause a double-free crash when both wrappers try to delete it.
    // Hint: Use '= delete;' on the copy constructor and copy assignment operator.
    UniqueHandle (const UniqueHandle&)            = delete;
    UniqueHandle (UniqueHandle&)                  = delete;
    UniqueHandle& operator= (const UniqueHandle&) = delete;
    UniqueHandle& operator= (UniqueHandle&)       = delete;
};

/*
// --- Step 3: Test your code ---
int main ()
{
    std::cout << "--- Starting Program ---\n";
    {
        // Creating a smart wrapper for dynamic integer memory
        UniqueHandle<int*, IntPointerDeleter> smartInt (new int (42));
        std::cout << "Value inside smartInt: " << *(smartInt.resource) << "\n";

        // Creating a smart wrapper for a file asset string
        UniqueHandle<const char*, FileDeleter> smartFile ("save_game.dat");
        std::cout << "Working with file: " << smartFile.resource << "\n";

        // 3. TODO: Un-comment the line below after writing step 2.
        // Your code should refuse to compile this line to protect against copy bugs!
        // UniqueHandle<const char*, FileDeleter> copyAttempt = smartFile;

        std::cout << "--- Leaving the inner scope block ---\n";
    }
    // <-- The wrappers go out of scope right here!
    // Your destructors should automatically trigger and print the cleanup messages.

    std::cout << "--- Program Finished Safely ---\n";
    return 0;
}
*/
