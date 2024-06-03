#pragma once
// NOLINTBEGIN
// #ifndef NAN
    #define NAN (__builtin_nanf(""))
// #endif

class Preferences
{
protected:
    unsigned int _handle;
    bool _started;
    bool _readOnly;

public:
    Preferences();
    ~Preferences();

    virtual inline bool begin(const char *name, bool readOnly = false, const char *partition_label = "a") { return true; }
    virtual inline void end() {}

    bool clear() { return true; }
    bool remove(const char *key) { return true; }

    virtual inline unsigned char putChar(const char *key, char value) { return 1; }
    virtual inline unsigned char putUChar(const char *key, unsigned char value) { return 1; }
    virtual inline unsigned char putShort(const char *key, short value) { return 1; }
    virtual inline unsigned char putUShort(const char *key, unsigned short value) { return 1; }
    virtual inline unsigned char putInt(const char *key, int value) { return 1; }
    virtual inline unsigned char putUInt(const char *key, unsigned int value) { return 1; }
    virtual inline unsigned char putLong(const char *key, int value) { return 1; }
    virtual inline unsigned char putULong(const char *key, unsigned int value) { return 1; }
    virtual inline unsigned char putLong64(const char *key, long value) { return 1; }
    virtual inline unsigned char putULong64(const char *key, unsigned long value) { return 1; }
    virtual inline unsigned char putFloat(const char *key, float value) { return 1; }
    virtual inline unsigned char putDouble(const char *key, double value) { return 1; }
    virtual inline unsigned char putBool(const char *key, bool value) { return 1; }
    virtual inline unsigned char putString(const char *key, const char *value) { return 1; }
    // unsigned char putString(const char *key, String value);
    virtual inline unsigned char putBytes(const char *key, const void *value, unsigned char len) { return 1; }

    virtual inline bool isKey(const char *key) { return true; }
    // PreferenceType getType(const char *key);
    virtual inline char getChar(const char *key, char defaultValue = 0) { return 1; }
    virtual inline unsigned char getUChar(const char *key, unsigned char defaultValue = 0) { return 1; }
    virtual inline short getShort(const char *key, short defaultValue = 0) { return 1; }
    virtual inline unsigned short getUShort(const char *key, unsigned short defaultValue = 0) { return 1; }
    virtual inline int getInt(const char *key, int defaultValue = 0) { return 1; }
    virtual inline unsigned int getUInt(const char *key, unsigned int defaultValue = 0) { return 1; }
    virtual inline int getLong(const char *key, int defaultValue = 0) { return 1; }
    virtual inline unsigned int getULong(const char *key, unsigned int defaultValue = 0) { return 1; }
    virtual inline long getLong64(const char *key, long defaultValue = 0) { return 1; }
    virtual inline unsigned long getULong64(const char *key, unsigned long defaultValue = 0) { return 1; }
    virtual inline float getFloat(const char *key, float defaultValue = NAN) { return 1; }
    virtual inline double getDouble(const char *key, double defaultValue = NAN) { return 1; }
    virtual inline bool getBool(const char *key, bool defaultValue = false) { return 1; }
    virtual inline unsigned char getString(const char *key, char *value, unsigned char maxLen) { return 1; }
    // String getString(const char *key, String defaultValue = String());
    virtual inline unsigned char getBytesLength(const char *key) { return 1; }
    virtual inline unsigned char getBytes(const char *key, void *buf, unsigned char maxLen) { return 1; }
    virtual inline unsigned char freeEntries() { return 1; }
};
// NOLINTEND
