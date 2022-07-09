#include <include.hpp>

void init_matrix(int, int, float *);
void print_matrix(int, int, float *, std::string);

int main() {
  // Settings
  constexpr uint N = 1024;
  constexpr uint B = 16;

  // Variables declaration
  float *matrix_a = (float *)malloc(sizeof(float) * N * N);
  float *matrix_b = (float *)malloc(sizeof(float) * N * N);
  float *matrix_c = (float *)malloc(sizeof(float) * N * N);

  // Init
  init_matrix(N, N, matrix_a);
  init_matrix(N, N, matrix_b);
  memset(matrix_c, 0, sizeof(float) * N * N);

  std::chrono::_V2::steady_clock::time_point start;

  // Device selector
  try {
#if DEVICE_VALUE == CPU_DEVICE
    sycl::queue q{sycl::cpu_selector()};
#elif DEVICE_VALUE == GPU_DEVICE
    sycl::queue q{sycl::gpu_selector()};
#elif DEVICE_VALUE == HOST_DEVICE
    sycl::queue q{sycl::host_selector()};
#endif

    // Buffers
    sycl::buffer<float, 2> buffer_a(matrix_a, sycl::range<2>{N, N});
    sycl::buffer<float, 2> buffer_b(matrix_b, sycl::range<2>{N, N});
    sycl::buffer<float, 2> buffer_r(matrix_c, sycl::range<2>{N, N});

    std::cout << "Execution on this device: " << q.get_device().get_info<sycl::info::device::name>() << "\n";

    // Queue
    q.submit([&](sycl::handler &cgh) {
      sycl::accessor<float, 2> acce_a{buffer_a, cgh, sycl::read_only};
      sycl::accessor<float, 2> acce_b{buffer_b, cgh, sycl::read_only};
      sycl::accessor<float, 2> acce_r{buffer_r, cgh, sycl::write_only, sycl::no_init};

      start = std::chrono::steady_clock::now();

      cgh.parallel_for(sycl::nd_range<2>{{N, N}, {B, B}}, [=](sycl::nd_item<2> idx) {
        const size_t r = idx.get_global_id(0);
        const size_t c = idx.get_global_id(1);

        float sum = 0.f;
        for (uint k = 0; k < N; k++) {
          sum += acce_a[r][k] * acce_b[k][c];
        }

        acce_r[r][c] = sum;
      });
    });

    q.wait();

  } catch (const sycl::exception &e) {
    std::cout << "Exception caught: " << e.what() << std::endl;
  }

  std::chrono::_V2::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::chrono::duration<double> elapsed_seconds = end - start;
  std::cout << "Time in seconds : " << elapsed_seconds.count() << std::endl;

  if (N < 10) {
    print_matrix(N, N, matrix_a, "MATRICE A");
    print_matrix(N, N, matrix_b, "MATRICE B");
    print_matrix(N, N, matrix_c, "MATRICE C");
  }

  free(matrix_a);
  free(matrix_b);
  free(matrix_c);
}

void init_matrix(int rows_size, int columns_size, float *matrix) {
  int i;

  for (i = 0; i < rows_size * columns_size; i++) {
    *(matrix + i) = std::rand() % 10 + 1;
  }
}

void print_matrix(int rows_size, int columns_size, float *matrix, std::string matrix_name) {
  int i;

  std::cout << std::endl << matrix_name << std::endl;
  for (i = 0; i < rows_size * columns_size; i++) {
    if (rows_size < 10) {
      printf("| ");
      std::cout << "| " << *(matrix + i) << " ";
    } else {
      std::cout << *(matrix + i) << " ";
    }

    if (rows_size < 10) {
      if ((i + 1) % columns_size == 0 && i != 0) std::cout << "| " << std::endl;
    } else {
      if ((i + 1) % columns_size == 0 && i != 0) std::cout << std::endl;
    }
  }
}