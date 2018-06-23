#include "bitmap.h"
#include <debug.h>
#include <limits.h>
#include <round.h>
#include <stdio.h>
#include "threads/malloc.h"
#include "threads/palloc.h"
#ifdef FILESYS
#include "filesys/file.h"
#endif

/* Element type.

   This must be an unsigned integer type at least as wide as int.

   Each bit represents one bit in the bitmap.
   If bit 0 in an element represents bit K in the bitmap,
   then bit 1 in the element represents bit K+1 in the bitmap,
   and so on. */
typedef unsigned long elem_type;

/* Number of bits in an element. */
#define ELEM_BITS (sizeof (elem_type) * CHAR_BIT)

/* From the outside, a bitmap is an array of bits.  From the
   inside, it's an array of elem_type (defined above) that
   simulates an array of bits. */
struct bitmap
  {
    size_t bit_cnt;     /* Number of bits. */
    elem_type *bits;    /* Elements that represent bits. */
  };

/* Returns the index of the element that contains the bit
   numbered BIT_IDX. */
static inline size_t
elem_idx (size_t bit_idx) 
{
  return bit_idx / ELEM_BITS;
}

/* Returns an elem_type where only the bit corresponding to
   BIT_IDX is turned on. */
static inline elem_type
bit_mask (size_t bit_idx) 
{
  return (elem_type) 1 << (bit_idx % ELEM_BITS);
}

/* Returns the number of elements required for BIT_CNT bits. */
static inline size_t
elem_cnt (size_t bit_cnt)
{
  return DIV_ROUND_UP (bit_cnt, ELEM_BITS);
}

/* Returns the number of bytes required for BIT_CNT bits. */
static inline size_t
byte_cnt (size_t bit_cnt)
{
  return sizeof (elem_type) * elem_cnt (bit_cnt);
}

/* Returns a bit mask in which the bits actually used in the last
   element of B's bits are set to 1 and the rest are set to 0. */
static inline elem_type
last_mask (const struct bitmap *b) 
{
  int last_bits = b->bit_cnt % ELEM_BITS;
  return last_bits ? ((elem_type) 1 << last_bits) - 1 : (elem_type) -1;
}

/* Creation and destruction. */

/* Creates and returns a pointer to a newly allocated bitmap with room for
   BIT_CNT (or more) bits.  Returns a null pointer if memory allocation fails.
   The caller is responsible for freeing the bitmap, with bitmap_destroy(),
   when it is no longer needed. */
struct bitmap *
bitmap_create (size_t bit_cnt) 
{
  struct bitmap *b = malloc (sizeof *b);
  if (b != NULL)
    {
      b->bit_cnt = bit_cnt;
      b->bits = malloc (byte_cnt (bit_cnt));
      if (b->bits != NULL || bit_cnt == 0)
        {
          bitmap_set_all (b, false);
          return b;
        }
      free (b);
    }
  return NULL;
}

/* Creates and returns a bitmap with BIT_CNT bits in the
   BLOCK_SIZE bytes of storage preallocated at BLOCK.
   BLOCK_SIZE must be at least bitmap_needed_bytes(BIT_CNT). */
struct bitmap *
bitmap_create_in_buf (size_t bit_cnt, void *block, size_t block_size UNUSED)
{
  struct bitmap *b = block;
  
  ASSERT (block_size >= bitmap_buf_size (bit_cnt));

  b->bit_cnt = bit_cnt;
  b->bits = (elem_type *) (b + 1);
  bitmap_set_all (b, false);
  return b;
}

/* Returns the number of bytes required to accomodate a bitmap
   with BIT_CNT bits (for use with bitmap_create_in_buf()). */
size_t
bitmap_buf_size (size_t bit_cnt) 
{
  return sizeof (struct bitmap) + byte_cnt (bit_cnt);
}

/* Destroys bitmap B, freeing its storage.
   Not for use on bitmaps created by bitmap_create_in_buf(). */
void
bitmap_destroy (struct bitmap *b) 
{
  if (b != NULL) 
    {
      free (b->bits);
      free (b);
    }
}

/* Bitmap size. */

/* Returns the number of bits in B. */
size_t
bitmap_size (const struct bitmap *b)
{
  return b->bit_cnt;
}

/* Setting and testing single bits. */

/* Atomically sets the bit numbered IDX in B to VALUE. */
void
bitmap_set (struct bitmap *b, size_t idx, bool value) 
{
  ASSERT (b != NULL);
  ASSERT (idx < b->bit_cnt);
  if (value)
    bitmap_mark (b, idx);
  else
    bitmap_reset (b, idx);
}

/* Atomically sets the bit numbered BIT_IDX in B to true. */
void
bitmap_mark (struct bitmap *b, size_t bit_idx) 
{
  size_t idx = elem_idx (bit_idx);
  elem_type mask = bit_mask (bit_idx);

  /* This is equivalent to `b->bits[idx] |= mask' except that it
     is guaranteed to be atomic on a uniprocessor machine.  See
     the description of the OR instruction in [IA32-v2b]. */
  asm ("orl %1, %0" : "=m" (b->bits[idx]) : "r" (mask) : "cc");
}

/* Atomically sets the bit numbered BIT_IDX in B to false. */
void
bitmap_reset (struct bitmap *b, size_t bit_idx) 
{
  size_t idx = elem_idx (bit_idx);
  elem_type mask = bit_mask (bit_idx);

  /* This is equivalent to `b->bits[idx] &= ~mask' except that it
     is guaranteed to be atomic on a uniprocessor machine.  See
     the description of the AND instruction in [IA32-v2a]. */
  asm ("andl %1, %0" : "=m" (b->bits[idx]) : "r" (~mask) : "cc");
}

/* Atomically toggles the bit numbered IDX in B;
   that is, if it is true, makes it false,
   and if it is false, makes it true. */
void
bitmap_flip (struct bitmap *b, size_t bit_idx) 
{
  size_t idx = elem_idx (bit_idx);
  elem_type mask = bit_mask (bit_idx);

  /* This is equivalent to `b->bits[idx] ^= mask' except that it
     is guaranteed to be atomic on a uniprocessor machine.  See
     the description of the XOR instruction in [IA32-v2b]. */
  asm ("xorl %1, %0" : "=m" (b->bits[idx]) : "r" (mask) : "cc");
}

/* Returns the value of the bit numbered IDX in B. */
bool
bitmap_test (const struct bitmap *b, size_t idx) 
{
  ASSERT (b != NULL);
  ASSERT (idx < b->bit_cnt);
  
  return (b->bits[elem_idx (idx)] & bit_mask (idx)) != 0;
}



/* Setting and testing multiple bits. */

/* Sets all bits in B to VALUE. */
void
bitmap_set_all (struct bitmap *b, bool value) 
{
  ASSERT (b != NULL);

  bitmap_set_multiple (b, 0, bitmap_size (b), value);
}

/* Sets the CNT bits starting at START in B to VALUE. */
void
bitmap_set_multiple (struct bitmap *b, size_t start, size_t cnt, bool value) 
{
  size_t i;
  
  ASSERT (b != NULL);
  ASSERT (start <= b->bit_cnt);
  ASSERT (start + cnt <= b->bit_cnt);

  for (i = 0; i < cnt; i++)
    bitmap_set (b, start + i, value);
}

/* Returns the number of bits in B between START and START + CNT,
   exclusive, that are set to VALUE. */
size_t
bitmap_count (const struct bitmap *b, size_t start, size_t cnt, bool value) 
{
  size_t i, value_cnt;

  ASSERT (b != NULL);
  ASSERT (start <= b->bit_cnt);
  ASSERT (start + cnt <= b->bit_cnt);

  value_cnt = 0;
  for (i = 0; i < cnt; i++)
    if (bitmap_test (b, start + i) == value)
      value_cnt++;
  return value_cnt;
}

/* Returns true if any bits in B between START and START + CNT,
   exclusive, are set to VALUE, and false otherwise. */
bool
bitmap_contains (const struct bitmap *b, size_t start, size_t cnt, bool value) 
{
  size_t i;
  
  ASSERT (b != NULL);
  ASSERT (start <= b->bit_cnt);
  ASSERT (start + cnt <= b->bit_cnt);

  for (i = 0; i < cnt; i++)
    if (bitmap_test (b, start + i) == value)
      return true;
  return false;
}

/* Returns true if any bits in B between START and START + CNT,
   exclusive, are set to true, and false otherwise.*/
bool
bitmap_any (const struct bitmap *b, size_t start, size_t cnt) 
{
  return bitmap_contains (b, start, cnt, true);
}

/* Returns true if no bits in B between START and START + CNT,
   exclusive, are set to true, and false otherwise.*/
bool
bitmap_none (const struct bitmap *b, size_t start, size_t cnt) 
{
  return !bitmap_contains (b, start, cnt, true);
}

/* Returns true if every bit in B between START and START + CNT,
   exclusive, is set to true, and false otherwise. */
bool
bitmap_all (const struct bitmap *b, size_t start, size_t cnt) 
{
  return !bitmap_contains (b, start, cnt, false);
}

void tree_print(size_t * tree){       // 사용 중인 영역을 확인하기 위하여 선언한 테스트 코드.
  printf("tree-------------------------\n");
  for(int i = 0 ;i<16 ; i++){
    for(int j =0 ; j<32; j++){
      printf("%d",tree[(i*32) + j]);
    }
    printf("\n");
  }
}
bool
buddy_remove (size_t start, size_t cnt) // free를 할때 cnt와 할당 받았던 index가 넘어온다.
{
  size_t binary_size = 1;             // 할당을 2의 제곱수 만큼 했기 때문에 마찬가지로 cnt보다 큰 2의 제곱수를 구한다.
    while(1){
      binary_size*=2;
      if(cnt <= binary_size)
      break;
    }

    if(cnt == 1){
      binary_size = 1;
    }

  for(int j = start; j< start + binary_size; j++){  // 다시 그 영역을 사용할 수 있으므로 색칠한 영역을 다시 초기화한다.
      tree[j] = 0;
  }

}

/* Finding set or unset bits. */

/* Finds and returns the starting index of the first group of CNT
   consecutive bits in B at or after START that are all set to
   VALUE.
   If there is no such group, returns BITMAP_ERROR. */
size_t
bitmap_scan (const struct bitmap *b, size_t start, size_t cnt, bool value) 
{
  
  ASSERT (b != NULL);
  ASSERT (start <= b->bit_cnt);
  
  static size_t recent = 0;     //가장 최근에 할당된 위치 (next fit 알고리즘을 위한 변수)
  static size_t check_cnt = 0;    //해당 함수가 실행되는 횟수
  static size_t check_size = 0;   //할당되는 공간의 크기 

  if(check_cnt < 3)
  {
    check_size = check_size + cnt;
  }
  if (cnt <= b->bit_cnt) 
    {
      size_t last = b->bit_cnt - cnt;
      size_t i;
    
    if(pallocator == 0)   //first fit 메모리 할당 알고리즘 호출
    {
      for (i = start; i <= last; i++){
        if (!bitmap_contains (b, i, cnt, !value))
          return i; 
    }
    }
    else if(pallocator == 1)    //next fit 메모리 할당 알고리즘 호출(-ma=1 입력시 nextfit 실행)
    {
      for(i = recent; i <= last; i++) { //가장 최근에 배치된 메모리 위치에서부터 마지막 위치까지 검색
        if (!bitmap_contains (b, i, cnt, !value))
        {
          recent = i;
          return i;
        }
      }
        for (i = start; i<= recent; i++)  { //메모리의 시작점인 위치부터 가장 최근에 배치된 메모리 위치까지 검색. 
                          //맨 마지막까지 할당받으면 nextfit은 직전에 할당했던곳 부터 할당하는데 마지막까지 할당하는 부분이 최근으로 참조했던 부분이니깐 다시 처음부터 다시 끝까지 탐색.
          if (!bitmap_contains (b, i, cnt, !value))
            
            {
              recent = i;
              return i;
            }   
        }
    }
    else if(pallocator == 2)      //best fit 메모리 할당 알고리즘 호출(-ma=2 입력시 bestfit 실행)
    {
      size_t idx = 0;       //fit될 수 있는 인덱스
      size_t tempIdx = 0;     //그 다음의 인덱스
      bool space = 0;       //이 변수에 할당가능한 인덱스들이 저장.
      size_t size = b->bit_cnt;   //공간 최대크기

      for(i= start; i <= last; i++){        //메모리 시작점 처음부터 끝까지 검색
        if(bitmap_test (b, start + i) == false) //만약 i bit가 0이면
        {
          if(space == 0)    //할당받을수 있는 첫번째 영역에 인덱스를 만남
          {
            tempIdx = i;
            if(idx == 0) idx = tempIdx;   // 뒷 부분에 할당된 공간이 없을 수도 있기 때문에 미리 인덱스를 저장.
          }
          space ++;       //공간에 할당가능한 그 다음의 인덱스가 증가
        }
        else  //만약 i bit가 1이면
        {
          if((space >= cnt)&&(space < size))
          {
            size = space;
            idx = tempIdx;        //인덱스는 현 인덱스로 저장
          }
          space = 0;      //1을 만나면 space를 0으로 초기화
        }
      }
      return idx;   //인덱스 반환
    }
      
  
    
  else if(pallocator == 3){
      size_t binary_size = 1;     // cnt보다 큰 2의 제곱수를 구하기 위하여 선언
      while(1){
        binary_size*=2;           // 2씩 곱하면서 2의 제곱으로 만든다.
        if(cnt <= binary_size)    // 2의 제곱수를 구하면 break;
        break;
      }

      if(cnt == 1){               // 시스템에서 필요로하는 cnt가 1개씩 3개를 받기때문에 2의 제곱수를 무시한다.
        binary_size = 1;
      }
    
    
      for (i = start; i <= last; i++){        // 처음부터 마지막 까지 검사를 한다.
          if (!bitmap_contains (b, i, binary_size, !value) && i%binary_size == 0 && tree[i] != 1){
            printf("idx : %d cnt : %d binary_size : %d\n", i , cnt, binary_size);
            // 2의 제곱수 만큼 할당 받을 수 있는 시작 index를 구할때, i가 구해진 2의 제곱수로 나눴을때 0이고 사용중인 영역(tree)가 1이 아니여야 그 영역을 사용가능하므로
            // 조건을 걸었다. buddy 시스템을 할때 0부터 구한 binary_size만큼 건너뛰면서 검사한다고 생각하면 된다.
            for(int j = i; j< i + binary_size; j++){    // 찾은 영역을 색칠한다.
              tree[j] = 1;
            }
            tree_print(tree);
            return i; 
          }
      }
  
   }
}
}


/* Finds the first gro
up of CNT consecutive bits in B at or after
   START that are all set to VALUE, flips them all to !VALUE,
   and returns the index of the first bit in the group.
   If there is no such group, returns BITMAP_ERROR.
   If CNT is zero, returns 0.
   Bits are set atomically, but testing bits is not atomic with
   setting them. */
size_t
bitmap_scan_and_flip (struct bitmap *b, size_t start, size_t cnt, bool value)
{
  size_t idx = bitmap_scan (b, start, cnt, value);
  if (idx != BITMAP_ERROR) 
    bitmap_set_multiple (b, idx, cnt, !value);
  return idx;
}

/* File input and output. */

#ifdef FILESYS
/* Returns the number of bytes needed to store B in a file. */
size_t
bitmap_file_size (const struct bitmap *b) 
{
  return byte_cnt (b->bit_cnt);
}

/* Reads B from FILE.  Returns true if successful, false
   otherwise. */
bool
bitmap_read (struct bitmap *b, struct file *file) 
{
  bool success = true;
  if (b->bit_cnt > 0) 
    {
      off_t size = byte_cnt (b->bit_cnt);
      success = file_read_at (file, b->bits, size, 0) == size;
      b->bits[elem_cnt (b->bit_cnt) - 1] &= last_mask (b);
    }
  return success;
}

/* Writes B to FILE.  Return true if successful, false
   otherwise. */
bool
bitmap_write (const struct bitmap *b, struct file *file)
{
  off_t size = byte_cnt (b->bit_cnt);
  return file_write_at (file, b->bits, size, 0) == size;
}
#endif /* FILESYS */

/* Debugging. */

/* Dumps the contents of B to the console as hexadecimal. */
void
bitmap_dump (const struct bitmap *b) 
{
  hex_dump (0, b->bits, byte_cnt (b->bit_cnt), false);
}

/* Dumps the contents of B to the console as binary. */
void
bitmap_dump2 (const struct bitmap *b)
{
  size_t i, j;

  for (i=0; i<elem_cnt (b->bit_cnt); i++) {
    for (j=0; j<ELEM_BITS; j++) {
      if ((i * ELEM_BITS + j) < b->bit_cnt) {
        printf ("%u", (unsigned int) (b->bits[i] >> j) & 0x1);
      }      
    }
    printf ("\n");
  }
}