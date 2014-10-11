#ifndef FunctionsAccessible_H__
# define FunctionsAccessible_H__

#include <vector>

#include "error.h"

#include "SFINAE.hh"
#include "unpacker.hh"

#include "Loadable.hh"

#include "typeToLoadable.hh"

namespace cTVScript {

  struct parametersPack{
    Loadable*			_this;
    Loadable*&			_return;
    std::vector<Loadable*>	_arguments;
  };

  class functionLoadable : public Loadable{
  public:
    NEGATE_VALUE_GETTER();
    virtual void call(parametersPack& pack) = 0;
    virtual void operator()(parametersPack& pack) {
      call(pack);
    }

  public:
    functionLoadable(const std::string& name)
      : Loadable(name) {}
  };

  template <typename Return, typename... Arguments>
  class StaticLoadableFunction : public functionLoadable{
  private:
    Return (*fn)(Arguments...);
  public:
    virtual void call(parametersPack& pack) {
      if (pack._arguments.size() < sizeof...(Arguments))
	throw cTVScript::MissingParameters(Helper::getTypeName<Return (*)(Arguments...)>(), this->name);
      else
	throw cTVScript::TooManyParameters(Helper::getTypeName<Return (*)(Arguments...)>(), this->name);

      DestructibleKey key = Key::create();
      pack._return = new typename Helper::DeduceLoadableType<Return>::type ("return",
									    Unpacker::applyFunction(key, pack._arguments, fn));
    }
    StaticLoadableFunction(const std::string& name,
			   Return (*_fn)(Arguments...))
      : functionLoadable(name), fn(_fn){}
  };

  template <typename Return, typename... Arguments>
  class StaticLoadableFunction<Return*, Arguments...> : public functionLoadable{
  private:
    Return (*fn)(Arguments...);
  public:
    virtual void call(parametersPack& pack) {
      if (pack._arguments.size() < sizeof...(Arguments))
	throw cTVScript::MissingParameters(Helper::getTypeName<Return (*)(Arguments...)>(), this->name);
      else
	throw cTVScript::TooManyParameters(Helper::getTypeName<Return (*)(Arguments...)>(), this->name);

      DestructibleKey key = Key::create();
      pack._return = dynamic_cast<Loadable*> (Unpacker::applyFunction(key, pack._arguments, fn));
    }
    StaticLoadableFunction(const std::string& name,
			   Return* (*_fn)(Arguments...))
      : functionLoadable(name), fn(_fn){}
  };

  template <typename... Arguments>
  class StaticLoadableFunction<void, Arguments...> : public functionLoadable{
  private:
    void (*fn)(Arguments...);
  public:
    virtual void call(parametersPack& pack) {
      if (pack._arguments.size() < sizeof...(Arguments))
	throw cTVScript::MissingParameters(Helper::getTypeName<void (*)(Arguments...)>(), this->name);
      else
	throw cTVScript::TooManyParameters(Helper::getTypeName<void (*)(Arguments...)>(), this->name);
      Unpacker::applyFunction(pack._arguments, fn);
    }
    StaticLoadableFunction(const std::string& name,
			   void (*_fn)(Arguments...))
      : functionLoadable(name), fn(_fn){}
  };

  template <typename Object, typename Return, typename... Arguments>
  class LoadableMethode : public functionLoadable {
  private:
    Return (Object::*fn)(Arguments...);
  public:
    virtual void call(parametersPack& pack) {
      if (pack._arguments.size() < sizeof...(Arguments))
	throw cTVScript::MissingParameters(Helper::getTypeName<Return (Object::*)(Arguments...)>(), this->name);
      else
	throw cTVScript::TooManyParameters(Helper::getTypeName<Return (Object::*)(Arguments...)>(), this->name);
      Object* _this = dynamic_cast<Object*> (pack._this);
      DestructibleKey key = Key::create();
      pack._return = new typename Helper::DeduceLoadableType<Return>::type ("return",
									    Unpacker::applyMethode(key, pack._arguments, _this, fn));
    }
    LoadableMethode(const std::string& name, Return (Object::*_fn)(Arguments...))
      : functionLoadable(name), fn(_fn){}
  };

  template <typename Object, typename Return, typename... Arguments>
  class LoadableMethode<Object, Return*, Arguments...> : public functionLoadable {
  private:
    Return (Object::*fn)(Arguments...);
  public:
    virtual void call(parametersPack& pack) {
      if (pack._arguments.size() < sizeof...(Arguments))
	throw cTVScript::MissingParameters(Helper::getTypeName<Return (Object::*)(Arguments...)>(), this->name);
      else
	throw cTVScript::TooManyParameters(Helper::getTypeName<Return (Object::*)(Arguments...)>(), this->name);
      Object* _this = dynamic_cast<Object*> (pack._this);
      DestructibleKey key = Key::create();
      pack._return = dynamic_cast<Loadable*> (Unpacker::applyMethode(key, pack._arguments, _this, fn));
    }
    LoadableMethode(const std::string& name, Return* (Object::*_fn)(Arguments...))
      : functionLoadable(name), fn(_fn){}
  };

  template <typename Object, typename... Arguments>
  class LoadableMethode<Object, void, Arguments...> : public functionLoadable{
  private:
    void (Object::*fn)(Arguments...);
  public:
    virtual void call(parametersPack& pack) {
      if (pack._arguments.size() < sizeof...(Arguments))
	throw cTVScript::MissingParameters(Helper::getTypeName<void (Object::*)(Arguments...)>(), this->name);
      else
	throw cTVScript::TooManyParameters(Helper::getTypeName<void (Object::*)(Arguments...)>(), this->name);
      Object* _this = dynamic_cast<Object*> (pack._this);
      DestructibleKey key = Key::create();
      Unpacker::applyMethode(key, pack._arguments, _this, fn);
    }
    LoadableMethode(const std::string& name, void (Object::*_fn)(Arguments...))
      : functionLoadable(name), fn(_fn){}
  };



  template<bool> struct makeLoadableMethode{};

  template<>
  struct makeLoadableMethode<true>{
    template<typename Return, typename Object, typename... Arguments>
    static Loadable* get(const std::string& name, Return (Object::*m)(Arguments...)) {
      return new LoadableMethode<Object, Return, Arguments...>(name, m);
    }
  };

  template<>
  struct makeLoadableMethode<false>{
    template<typename Return, typename Object, typename... Arguments>
    static Loadable* get(const std::string&, Return (Object::*)(Arguments...)) {
      static_assert(SFINAE::is_Loadable<Object*>::value == true, "[cTVScript error] Loadable Methode class have to herite from Loadable");
      return NULL;
    }
  };

  template<typename Return, typename Object, typename... Arguments>
  Loadable* makeMethodeLoadable(const std::string& name, Return (Object::*m)(Arguments...)) {
    return makeLoadableMethode<SFINAE::is_Loadable<Object*>::value >::get(name, m);
  }

  template<typename Return, typename... Arguments>
  Loadable* makeFunctionLoadable(const std::string& name, Return (*m)(Arguments...)) {
    return new StaticLoadableFunction<Return, Arguments...>(name, m);
  }
};

#endif