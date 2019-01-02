# Threading
Little tool allowing me to add multithreading with a few lines

The main goal for this tool is to allow for very rapid parallism prototyping, or "threading in under a minute".

Questions to Address (NOTE TO SELF: after addressed, write tests for these cases):
1. At the moment, can request a status change to API, but how can I guarantee that the worker will see the status request if it is overwritten?
   - Three options off the top of my head:
   	- Ignore _new_ request if old has not been acknowledged
   	  - benefits: easy, fast, low memory footprint (probably doesn't matter but good to note), not hard to understand
   	  - drawbacks: nondeterministic behaviour, complicates user implementation
   	- lock until request is acknowledged:
   	  - benefits: easy, deterministic, low memory fooprint
      - drawbacks: slow, potentially very dangerous if worker in infinite loop
    - Have request queue to bank requests
      - benefits: deterministic, simplifies user code, greater api flexibility
      - drawbacks: complicates implementation, potentially large memory footprint
    - Allow for forcing requests:
      - benefits: simple to implement on worker side, fast, depending upon the implmentation may have a large memory footprint (if operating on copy of data to allow for safe abortion)
      - drawbacks: nondeterministic, potentially complicates either user code or implementation when addressing aborted tasks
2. How to handle potential for workers to be in infinite loops?
   - Right now, an infinite loop in the workers hangs the program when trying to retire thread pool
     - Allow for interrupt callback or something? This is not a solvable problem in the general case but should still be consciously addressed  
   - Options:
     - As hinted at above: interrupt callback to force a worker status change
       - benefits: relatively simple addresses the problem of the pool hanging currently
       - drawbacks: complicates API and implementation, can be abused (which means it will be abused)
     - Tell user that an infinite loop will be bad in documentation, and leave it at that
       - benefits: easiest solution, answers the question: "is it reasonable for me to have to worry about this?" With a resounding "NO"
       - drawbacks: doesn't actually solve the problem, just brushes it under the rug
     - Have a force-exit function, keep it internal, and have an option when retiring to force
       - benefits: Still pretty easy, actually allows for some kind of control over the situation, actually addresses problem
       - drawbacks: Slighly complicates API, can (will) be abused, more complexity internally
3. TBD

The next steps are as follows:
1. Begin Writing test suite
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
4. General Goals:
   - Currently 800 lines with lots of debug statements involved
   - I'd like to get it down to under 100 meaningful lines without debug statements or data structures
   - Unity build option, 1 include to facilitate under-a-minute goal
   - Nicely organized file structure for more permanent implementations
   - Pie-In-The-Sky: Robust enough that I can allow users to implement their own data structures, and set options with #defines

That's all I can think of for now...