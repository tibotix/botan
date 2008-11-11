/**
* Engine Base Class
* (C) 1999-2007 Jack Lloyd
*/

#include <botan/engine.h>
#include <botan/stl_util.h>
#include <botan/mode_pad.h>

namespace Botan {

namespace {

/*************************************************
* Algorithm Cache                                *
*************************************************/
template<typename T>
class Algorithm_Cache_Impl : public Engine::Algorithm_Cache<T>
   {
   public:
      T* get(const std::string& name) const
         {
         Mutex_Holder lock(mutex);
         return search_map(mappings, name);
         }

      void add(T* algo, const std::string& index_name = "") const
         {
         if(!algo)
            return;

         Mutex_Holder lock(mutex);

         const std::string name =
            (index_name != "" ? index_name : algo->name());

         if(mappings.find(name) != mappings.end())
            delete mappings[name];
         mappings[name] = algo;
         }

      Algorithm_Cache_Impl(Mutex* m) : mutex(m) {}

      ~Algorithm_Cache_Impl()
         {
         typename std::map<std::string, T*>::iterator i = mappings.begin();

         while(i != mappings.end())
            {
            delete i->second;
            ++i;
            }
         delete mutex;
         }
   private:
      Mutex* mutex;
      mutable std::map<std::string, T*> mappings;
   };

}

/*************************************************
* Acquire a BlockCipher                          *
*************************************************/
const BlockCipher*
Engine::prototype_block_cipher(const SCAN_Name& request,
                               Algorithm_Factory& af) const
   {
   // This needs to respect provider settings
   BlockCipher* algo = cache_of_bc->get(request.as_string());
   if(algo)
      return algo;

   // cache miss: do full search
   algo = find_block_cipher(request, af);
   if(algo)
      cache_of_bc->add(algo, request.as_string());

   return algo;
   }

/*************************************************
* Acquire a StreamCipher                         *
*************************************************/
const StreamCipher*
Engine::prototype_stream_cipher(const SCAN_Name& request,
                                Algorithm_Factory& af) const
   {
   // This needs to respect provider settings
   StreamCipher* algo = cache_of_sc->get(request.as_string());
   if(algo)
      return algo;

   // cache miss: do full search
   algo = find_stream_cipher(request, af);
   if(algo)
      cache_of_sc->add(algo, request.as_string());

   return algo;
   }

/*************************************************
* Acquire a HashFunction                         *
*************************************************/
const HashFunction*
Engine::prototype_hash_function(const SCAN_Name& request,
                                Algorithm_Factory& af) const
   {
   // This needs to respect provider settings
   HashFunction* algo = cache_of_hf->get(request.as_string());
   if(algo)
      return algo;

   // cache miss: do full search
   algo = find_hash(request, af);
   if(algo)
      cache_of_hf->add(algo, request.as_string());

   return algo;
   }

/*************************************************
* Acquire a MessageAuthenticationCode            *
*************************************************/
const MessageAuthenticationCode*
Engine::prototype_mac(const SCAN_Name& request,
                      Algorithm_Factory& af) const
   {
   // This needs to respect provider settings
   MessageAuthenticationCode* algo = cache_of_mac->get(request.as_string());
   if(algo)
      return algo;

   // cache miss: do full search
   algo = find_mac(request, af);
   if(algo)
      cache_of_mac->add(algo, request.as_string());

   return algo;
   }

/*************************************************
* Add a block cipher to the lookup table         *
*************************************************/
void Engine::add_algorithm(BlockCipher* algo) const
   {
   cache_of_bc->add(algo);
   }

/*************************************************
* Add a stream cipher to the lookup table        *
*************************************************/
void Engine::add_algorithm(StreamCipher* algo) const
   {
   cache_of_sc->add(algo);
   }

/*************************************************
* Add a hash function to the lookup table        *
*************************************************/
void Engine::add_algorithm(HashFunction* algo) const
   {
   cache_of_hf->add(algo);
   }

/*************************************************
* Add a MAC to the lookup table                  *
*************************************************/
void Engine::add_algorithm(MessageAuthenticationCode* algo) const
   {
   cache_of_mac->add(algo);
   }

void Engine::initialize(Mutex_Factory& mf)
   {
   cache_of_bc = new Algorithm_Cache_Impl<BlockCipher>(mf.make());
   cache_of_sc = new Algorithm_Cache_Impl<StreamCipher>(mf.make());
   cache_of_hf = new Algorithm_Cache_Impl<HashFunction>(mf.make());
   cache_of_mac =
      new Algorithm_Cache_Impl<MessageAuthenticationCode>(mf.make());
   }

Engine::Engine()
   {
   cache_of_bc = 0;
   cache_of_sc = 0;
   cache_of_hf = 0;
   cache_of_mac = 0;
   }

/*************************************************
* Destroy an Engine                              *
*************************************************/
Engine::~Engine()
   {
   delete cache_of_bc;
   delete cache_of_sc;
   delete cache_of_hf;
   delete cache_of_mac;
   }

}
