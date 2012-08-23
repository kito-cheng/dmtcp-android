#include <stdio.h>
#include <unistd.h>
extern "C" {
ssize_t tls_write(int fd, const void *buf, size_t count);
}

namespace dmtcp {
  class TLSUtils {
    public:
      typedef void *(*pthread_getspecific_t)(pthread_key_t);
      typedef int (*pthread_setspecific_t)(pthread_key_t, const void *);
      typedef int (*pthread_key_create_t)(pthread_key_t *,
                                          void (*dtor)(void *));
      typedef int (*pthread_key_delete_t)(pthread_key_t);

      static void *pthread_getspecific(pthread_key_t key);
      static int pthread_setspecific(pthread_key_t key, const void *value);
      static int pthread_key_create(pthread_key_t *key,
                                    void (*dtor)(void *));
      static int pthread_key_delete(pthread_key_t key);
      static void debug(const char *str);
    private:
      static TLSUtils &instance();
      TLSUtils();

      pthread_getspecific_t _pthread_getspecific;
      pthread_setspecific_t _pthread_setspecific;
      pthread_key_create_t _pthread_key_create;
      pthread_key_delete_t _pthread_key_delete;
  };
}

namespace dmtcp {
  template<class T>
  TLS<T>::TLS()
    : _tlsKey(0)
    , _default()
  {
    TLSUtils::pthread_key_create(&_tlsKey, NULL);
  }

  template<class T>
  TLS<T>::TLS(const T &val)
    : _tlsKey(0)
    , _default(val)
  {
    TLSUtils::pthread_key_create(&_tlsKey, NULL);
  }

  template<class T>
  TLS<T>::~TLS() {
    T *data = get();
    TLSUtils::pthread_key_delete(_tlsKey);
  }

  template<class T>
  T *TLS<T>::get() {
    T *data = (T *)TLSUtils::pthread_getspecific(_tlsKey);
    if (data == NULL) {
      data = TLS<T>::malloc();
      *data = _default;
      TLSUtils::pthread_setspecific(_tlsKey, (void*)data);
    }
    return data;
  }

  template<class T>
  T &TLS<T>::operator=(const T &val) {
    T *data = get();
    *data = val;
    return *data;
  }

  template<class T>
  TLS<T>::operator T&() {
    T *data = get();
    return *data;
  }

  template<class T>
  const T *TLS<T>::get() const {
    T *data = (T *)TLSUtils::pthread_getspecific(_tlsKey);
    if (data == NULL) {
      data = TLS<T>::malloc();
      *data = _default;
      TLSUtils::pthread_setspecific(_tlsKey, (void*)data);
    }
    return data;
  }

  template<class T>
  const T &TLS<T>::operator=(const T &val) const {
    const T *data = get();
    *data = val;
    return *data;
  }

  template<class T>
  TLS<T>::operator const T&() const {
    const T *data = get();
    return *data;
  }

  template<class T>
  T *TLS<T>::malloc() {
    static int idx = 0;
    static T tls_storage[1024];
    return &tls_storage[idx++];
  }
}
