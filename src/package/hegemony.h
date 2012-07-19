#ifndef HEGEMONY_H
#define HEGEMONY_H

#include "standard.h"


class TripleAngleBlade: public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE TripleAngleBlade(Card::Suit suit, int number);
};

class WuSixCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE WuSixCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class WuSixSwords: public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE WuSixSwords(Card::Suit suit, int number);
    virtual void onMove(const CardMoveStruct &move) const;
};


class AttackExhausted : public TrickCard{
    Q_OBJECT

public:
    Q_INVOKABLE AttackExhausted (Card::Suit suit, int number);

    virtual QString getSubtype() const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class KnowAll: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE KnowAll(Card::Suit suit, int number);

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;

    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class BefriendDistant: public SingleTargetTrick{
    Q_OBJECT

public:
    Q_INVOKABLE BefriendDistant(Card::Suit suit, int number);
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};


class FenxunCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE FenxunCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class ShuangrenCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE ShuangrenCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class HuoshuiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE HuoshuiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class QingchengCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE QingchengCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};

class XiongyiCard: public SkillCard{
    Q_OBJECT

public:
    Q_INVOKABLE XiongyiCard();
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};


class HegemonyCardPackage: public Package{
    Q_OBJECT

public:
    HegemonyCardPackage();
};

class HegemonyPackage: public Package{
    Q_OBJECT

public:
    HegemonyPackage();
};

#endif // HEGEMONY_H
