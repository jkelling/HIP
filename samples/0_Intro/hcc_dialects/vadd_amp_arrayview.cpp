// Simple test showing how to use C++AMP syntax with array_view.
// The code uses AMP's array_view class, which provides automatic data synchronization
// of data between the host and the accelerator. As noted below, the HCC runtime
// will automatically copy data to and from the host, without the user needing
// to manually perform such copies.  This is an excellent mode for developers
// new to GPU programming and matches the memory models provided by recent systems where
// CPU and GPU share the same memory pool.  Advanced programmers may prefer 
// more explicit control over the data movement - shown in the other vadd_hc_array and 
// vadd_hc_am examples.
// This example shows the similarity between C++AMP and and HC for simple cases where
// implicit data transfer is used - really the only difference is the namespace.
// Other examples show some of the more advanced controls.

#include <amp.h>

int main(int argc, char *argv[])
{
    int sizeElements = 1000000;
    bool pass = true;

    // Allocate auto-managed host/device views of data:
    concurrency::array_view<float> A(sizeElements);
    concurrency::array_view<float> B(sizeElements);
    concurrency::array_view<float> C(sizeElements);

    // Initialize host data
    for (int i=0; i<sizeElements; i++) {
        A[i] = 1.618f * i; 
        B[i] = 3.142f * i;
    }
    C.discard_data(); // tell runtime not to copy CPU host data.


    // Launch kernel onto default accelerator
    // The HCC runtime will ensure that A and B are available on the accelerator before launching the kernel.
    concurrency::parallel_for_each(concurrency::extent<1> (sizeElements),
      [=] (concurrency::index<1> idx) restrict(amp) { 
        int i = idx[0];
        C[i] = A[i] + B[i];
    });

    for (int i=0; i<sizeElements; i++) {
        float ref= 1.618f * i + 3.142f * i;
        // Because C is an array_view, the HCC runtime will copy C back to host at first access here:
        if (C[i] != ref) {
            printf ("error:%d computed=%6.2f, reference=%6.2f\n", i, C[i], ref);
            pass = false;
        }
    };
    if (pass) printf ("PASSED!\n");
}
