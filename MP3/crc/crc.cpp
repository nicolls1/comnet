#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <time.h>

using namespace std;

#define GENERATOR_LENGTH 4
#define LOOKUP_LENGTH 8192

long generate_crc(short generator, int in_bits, long data) {
  long crc = data << GENERATOR_LENGTH - 1;

  long shift_gen = 0;
  for(int i = in_bits + GENERATOR_LENGTH-1; i >= GENERATOR_LENGTH; --i) {
    shift_gen = generator << i - GENERATOR_LENGTH;
    //cout << hex << shift_gen << dec << " i: " << i << endl;
    //cout << "Bit check: " << (crc & (1 << i-1)) << endl;
    if(crc & (1 << i-1)) {
      crc ^= shift_gen;
    }
    //cout << "CRC: " << crc << endl;
  }

  return crc;
}

void generate_table(short table[], const short& generator) {
  for(int i = 0; i < (1 << 16); ++i) {
    table[i] = generate_crc(generator, 16, i);
  }
}

void get_random(short lookups[]) {
  srand(time(NULL));
  int max = (1<<16);
  for(int i = 0; i < LOOKUP_LENGTH; ++i) {
    lookups[i] = rand() % max;
  }
}

unsigned long getTime() {
  return chrono::system_clock::now().time_since_epoch()/chrono::nanoseconds(1);//microseconds(1);
}

// length is 11, and all zeros
// prob must be less than or equal to 100
int make_bit_error(int prob) {
  srand(getTime());
  int ret = 0;
  for(int i = 0; i < 11; ++i) {
    if((rand() % 100) < prob) {
      ret |= (1<<i);
    }
  }
  return ret;
}

int main(int argc, char* argv[]) {
  if(argc != 2 && argc != 4) {
    cout << "usage: encoder <generator> <data length> <data>" << endl;
  }

  if(argc == 4) {
    cout << "CRC: " << generate_crc(atoi(argv[1]), atoi(argv[2]), atoi(argv[3])) << endl;
  }

  if(argc == 2) {
    short table[1<<16];
    short lookups[LOOKUP_LENGTH];
    generate_table(table, atoi(argv[1]));
    get_random(lookups);

    short results[LOOKUP_LENGTH];
    short table_results[LOOKUP_LENGTH];

    unsigned long start_time = getTime();
    
    for(int i = 0; i < LOOKUP_LENGTH; ++i) {
      results[i] = generate_crc(atoi(argv[1]), 16, lookups[i]);
    }

    unsigned long end_time = getTime();

    unsigned long table_start_time = getTime();

    for(int i = 0; i < LOOKUP_LENGTH; ++i) {
      table_results[i] = table[lookups[i]];
    }

    unsigned long table_end_time = getTime();

    cout << "Times are in nanoseconds" << endl;
    cout << "Encoded start: " << start_time << endl;
    cout << "Encoded end: " << end_time << endl;
    cout << "Durration: " << end_time - start_time << endl;
    cout << "Average: " << (end_time - start_time) / LOOKUP_LENGTH << endl;

    cout << "Table start: " << table_start_time << endl;
    cout << "Table end: " << table_end_time << endl;
    cout << "Durration: " << table_end_time - table_start_time << endl;
    cout << "Average: " << (table_end_time - table_start_time) / LOOKUP_LENGTH << endl;

    cout << "Bit error: " << hex << make_bit_error(10) << dec << endl;

    int count = 0;
    int temp;
    for(int i = 0; i < LOOKUP_LENGTH; ) {
      temp = make_bit_error(10);
      //cout << hex << temp << dec << endl;
      if(temp == 0) {
        continue;
      }
      
      if(generate_crc(9, 11, temp) != 0) {
        ++count;
      }
      ++i;
    }

    cout << "10 error: " << count << " out of " << LOOKUP_LENGTH << endl;

    count = 0;
    for(int i = 0; i < LOOKUP_LENGTH; ) {
      temp = make_bit_error(20);
      if(temp == 0) {
        continue;
      }
      
      if(generate_crc(9, 11, temp) != 0) {
        ++count;
      }
      ++i;
    }

    cout << "20 error: " << count << " out of " << LOOKUP_LENGTH << endl;

    count = 0;
    for(int i = 0; i < LOOKUP_LENGTH; ) {
      temp = make_bit_error(30);
      if(temp == 0) {
        continue;
      }
      
      if(generate_crc(9, 11, temp) != 0) {
        ++count;
      }
      ++i;
    }

    cout << "30 error: " << count << " out of " << LOOKUP_LENGTH << endl;
  }
}
