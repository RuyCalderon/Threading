# Threading
Little tool allowing me to add multithreading with a few lines

The main goal for this tool is to allow for very rapid parallism prototyping, or "threading in under a minute".

The next steps are as follows:
1. Write tests
2. Go back over data structures
   - Stack implementations are currently placeholder
   - Selection of more appropriate data structures when needed
     - FIFO task queue is first and foremost:
       - Currently there is no guarantee that a task will ever be executed for long-running programs
    - Add some concurrent data structures for use in user-defined functions (optional)
      - Concurrent Queue, 
      - FIFO
      - LIFO
      - Concurrent Hash Table
    - Concurrent Linked List
      - Fine-grained (for FIFO task queue)
      - Course-grained (for use in hash table more than anything)
3. Refine API
   - Callbacks handled strangely
     - I'd like to construct a task peacemeal
       - Actual function
       - Callback (OnComplete)
       - Startup??(OnStart)
     - I'd also like the API to be simple
     - These two things seem like a tradeoff so there's a middle ground to find
   - Loooots to do internally, this step will likely prompt rewrites of data structure code and vice-versa
   - In terms of overall flow, tentative intention is to treat a task like a GPU Kernel, but not completely opaque
     - Flow 
       - Onstart - transform input data into parallel format
       - Task - Operate on data
       - OnComplete - transform parallel data back to input format
     - Complications:
       - All functions user defined, so renaming pass at end to make intention clear
4. (General Goal) Length requirements
   - Currently 800 lines with lots of debug statements involved
   - I'd like to get it down to under 100 meaningful lines without debug statements or data structures
   - Unity build option, 1 include to facilitate under-a-minute goal
   - Nicely organized file structure for more permanent implementations
   - Pie-In-The-Sky: Robust enough that I can allow users to implement their own data structures, and set options with #defines

That's all I can think of for now...
