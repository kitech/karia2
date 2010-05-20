// from http://websvn.kde.org/trunk/KDE/kdelibs/kdeui/itemviews/kbihash_p.h?view=markup

#ifndef QBIHASH_P_H
#define QBIHASH_P_H

#include <QtCore/QHash>
#include <QtCore/QDebug>

template<typename T, typename U> class QBiHash;
template<typename T, typename U> QDebug operator<<(QDebug out, const QBiHash<T, U> &biHash);
template<typename T, typename U> QDataStream &operator<<(QDataStream &out, const QBiHash<T, U> &bihash);
template<typename T, typename U> QDataStream &operator>>(QDataStream &in, QBiHash<T, U> &biHash);

/**
 * @brief QBiHash provides a bi-directional hash container
 *
 * @note This class is designed to make mapping easier in proxy model implementations.
 *
 * @todo Figure out whether to discard this and use boost::bimap instead, submit it Qt or keep it here and make more direct use of QHashNode.
 */
template<typename T, typename U>
class QBiHash
{
public:
    typedef T left_type;
    typedef U right_type;

    template<typename V, typename W>
    class _iterator : public QHash<V, W>::iterator
    {
    public:
        explicit inline _iterator(void *data) : QHash<V, W>::iterator(data) {}
        /* implicit */ _iterator(const typename QHash<V, W>::iterator it)
                  // Using internals here because I was too lazy to write my own iterator.
                : QHash<V, W>::iterator(reinterpret_cast<void *>(static_cast<QHashNode<V, W> *>(it))) { }

        inline const W &value() const {
            return QHash<V, W>::iterator::value();
        }
        inline const W &operator*() const {
            return QHash<V, W>::iterator::operator*();
        }
        inline const W *operator->() const {
            return QHash<V, W>::iterator::operator->();
        }

    private:
#ifndef Q_CC_MSVC
        using QHash<V, W>::iterator::operator*;
        using QHash<V, W>::iterator::operator->;
        using QHash<V, W>::iterator::value;
#endif
    };

    typedef _iterator<T, U>                      left_iterator;
    typedef typename QHash<T, U>::const_iterator left_const_iterator;
    typedef _iterator<U, T>                      right_iterator;
    typedef typename QHash<U, T>::const_iterator right_const_iterator;

    inline QBiHash() {}
    inline QBiHash(const QBiHash<T, U> &other) {
        *this = other;
    }

    static QBiHash<T, U> fromHash(const QHash<T, U> &hash) {
        QBiHash<T, U> biHash;
        typename QHash<T, U>::const_iterator it = hash.constBegin();
        const typename QHash<T, U>::const_iterator end = hash.constEnd();
        for ( ; it != end; ++it)
            biHash.insert(it.key(), it.value());
        return biHash;
    }

    const QBiHash<T, U> &operator=(const QBiHash<T, U> &other) {
        _leftToRight = other._leftToRight; _rightToLeft = other._rightToLeft; return *this;
    }

    inline bool removeLeft(T t) {
        const U u = _leftToRight.take(t);
        return _rightToLeft.remove(u) != 0;
    }

    inline bool removeRight(U u) {
        const T t = _rightToLeft.take(u);
        return _leftToRight.remove(t) != 0;
    }

    inline U takeLeft(T t) {
        const U u = _leftToRight.take(t);
        _rightToLeft.remove(u);
        return u;
    }

    inline T takeRight(U u) {
        const T t = _rightToLeft.take(u);
        _leftToRight.remove(t);
        return t;
    }

    inline T rightToLeft(U u) const {
        return _rightToLeft.value(u);
    }

    inline U leftToRight(T t) const {
        return _leftToRight.value(t);
    }

    inline bool leftContains(T t) const {
        return _leftToRight.contains(t);
    }

    inline bool rightContains(U u) const {
        return _rightToLeft.contains(u);
    }

    inline int size() const {
        return _leftToRight.size();
    }

    inline int count() const {
        return _leftToRight.count();
    }

    inline int capacity() const {
        return _leftToRight.capacity();
    }

    void reserve(int size) {
        _leftToRight.reserve(size); _rightToLeft.reserve(size);
    }

    inline void squeeze() {
        _leftToRight.squeeze(); _rightToLeft.squeeze();
    }

    inline void detach() {
        _leftToRight.detach(); _rightToLeft.detach();
    }

    inline bool isDetached() const {
        return _leftToRight.isDetached();
    }

    inline void setSharable(bool sharable) {
        _leftToRight.setSharable(sharable); _rightToLeft.setSharable(sharable);
    }

    inline bool isSharedWith(const QBiHash<T, U> &other) const {
        return _leftToRight.isSharedWith(other._leftToRight) && _rightToLeft.isSharedWith(other._leftToRight);
    }

    void clear() {
        _leftToRight.clear(); _rightToLeft.clear();
    }

    QList<T> leftValues() const {
        return _leftToRight.keys();
    }

    QList<U> rightValues() const {
        return _rightToLeft.keys();
    }

    right_iterator eraseRight(right_iterator it) {
        Q_ASSERT(it != rightEnd());
        _leftToRight.remove(it.value());
        return _rightToLeft.erase(it);
    }

    left_iterator eraseLeft(left_iterator it) {
        Q_ASSERT(it != leftEnd());
        _rightToLeft.remove(it.value());
        return _leftToRight.erase(it);
    }

    left_iterator findLeft(T t) {
        return _leftToRight.find(t);
    }

    left_const_iterator findLeft(T t) const {
        return _leftToRight.find(t);
    }

    left_const_iterator constFindLeft(T t) const {
        return _leftToRight.constFind(t);
    }

    right_iterator findRight(U u) {
        return _rightToLeft.find(u);
    }

    right_const_iterator findRight(U u) const {
        return _rightToLeft.find(u);
    }

    right_const_iterator constFindRight(U u) const {
        return _rightToLeft.find(u);
    }

    left_iterator insert(T t, U u) {
        // biHash.insert(5, 7); // creates 5->7 in _leftToRight and 7->5 in _rightToLeft
        // biHash.insert(5, 9); // replaces 5->7 with 5->9 in _leftToRight and inserts 9->5 in _rightToLeft.
        // The 7->5 in _rightToLeft would be dangling, so we remove it before insertion.

        // This means we need to hash u and t up to twice each. Could probably be done better using QHashNode.

        if (_leftToRight.contains(t))
            _rightToLeft.remove(_leftToRight.take(t));
        if (_rightToLeft.contains(u))
            _leftToRight.remove(_rightToLeft.take(u));

        _rightToLeft.insert(u, t);
        return _leftToRight.insert(t, u);
    }

    QBiHash<T, U> &intersect(const QBiHash<T, U> &other) {
        typename QBiHash<T, U>::left_iterator it = leftBegin();
        while (it != leftEnd()) {
            if (!other.leftContains(it.key()))
                it = eraseLeft(it);
            else
                ++it;
        }
        return *this;
    }

    QBiHash<T, U> &subtract(const QBiHash<T, U> &other) {
        typename QBiHash<T, U>::left_iterator it = leftBegin();
        while (it != leftEnd()) {
            if (other._leftToRight.contains(it.key()))
                it = eraseLeft(it);
            else
                ++it;
        }
        return *this;
    }

    QBiHash<T, U> &unite(const QBiHash<T, U> &other) {
        typename QHash<T, U>::const_iterator it = other._leftToRight.constBegin();
        const typename QHash<T, U>::const_iterator end = other._leftToRight.constEnd();
        while (it != end) {
            const T key = it.key();
            if (!_leftToRight.contains(key))
                insert(key, it.value());
            ++it;
        }
        return *this;
    }

    void updateRight(left_iterator it, U u) {
        Q_ASSERT(it != leftEnd());
        const T key = it.key();
        _rightToLeft.remove(_leftToRight.value(key));
        _leftToRight[key] = u;
        _rightToLeft[u] = key;
    }

    void updateLeft(right_iterator it, T t) {
        Q_ASSERT(it != rightEnd());
        const U key = it.key();
        _leftToRight.remove(_rightToLeft.value(key));
        _rightToLeft[key] = t;
        _leftToRight[t] = key;
    }

    inline bool isEmpty() const {
        return _leftToRight.isEmpty();
    }

    const U operator[](const T &t) const {
        return _leftToRight.operator[](t);
    }

    bool operator==(const QBiHash<T, U> &other) {
        return _leftToRight.operator == (other._leftToRight);
    }

    bool operator!=(const QBiHash<T, U> &other) {
        return _leftToRight.operator != (other._leftToRight);
    }

    left_iterator toLeftIterator(right_iterator it) const {
        Q_ASSERT(it != rightEnd());
        return _leftToRight.find(it.value());
    }

    right_iterator toRightIterator(left_iterator it) const {
        Q_ASSERT(it != leftEnd());
        return _rightToLeft.find(it.value());
    }

    inline left_iterator leftBegin() {
        return _leftToRight.begin();
    }

    inline left_iterator leftEnd() {
        return _leftToRight.end();
    }

    inline left_const_iterator leftBegin() const {
        return _leftToRight.begin();
    }

    inline left_const_iterator leftEnd() const {
        return _leftToRight.end();
    }

    inline left_const_iterator leftConstBegin() const {
        return _leftToRight.constBegin();
    }

    inline left_const_iterator leftConstEnd() const {
        return _leftToRight.constEnd();
    }

    inline right_iterator rightBegin() {
        return _rightToLeft.begin();
    }

    inline right_iterator rightEnd() {
        return _rightToLeft.end();
    }

    inline right_const_iterator rightBegin() const {
        return _rightToLeft.begin();
    }

    inline right_const_iterator rightEnd() const {
        return _rightToLeft.end();
    }
    inline right_const_iterator rightConstBegin() const {
        return _rightToLeft.constBegin();
    }

    inline right_const_iterator rightConstEnd() const {
        return _rightToLeft.constEnd();
    }

    friend QDataStream &operator<< <T, U>(QDataStream &out, const QBiHash<T, U> &bihash);
    friend QDataStream &operator>> <T, U>(QDataStream &in, QBiHash<T, U> &biHash);
    friend QDebug operator<< <T, U>(QDebug out, const QBiHash<T, U> &biHash);
private:
    QHash<T, U> _leftToRight;
    QHash<U, T> _rightToLeft;
};

template<typename T, typename U>
QDataStream &operator<<(QDataStream &out, const QBiHash<T, U> &biHash)
{
    return out << biHash._leftToRight;
}

template<typename T, typename U>
QDataStream &operator>>(QDataStream &in, QBiHash<T, U> &biHash)
{
    QHash<T, U> leftToRight;
    in >> leftToRight;
    typename QHash<T, U>::const_iterator it = leftToRight.constBegin();
    const typename QHash<T, U>::const_iterator end = leftToRight.constEnd();
    for (; it != end; ++it)
        biHash.insert(it.key(), it.value());

    return in;
}

template<typename T, typename U>
QDebug operator<<(QDebug out, const QBiHash<T, U> &biHash)
{
    typename QBiHash<T, U>::left_const_iterator it = biHash.leftConstBegin();

    const typename QBiHash<T, U>::left_const_iterator end = biHash.leftConstEnd();
    out << "QBiHash(";
    for (; it != end; ++it)
        out << "(" << it.key() << "<=>" << it.value() << ")";

    out << ")";
    return out;
}

#endif
