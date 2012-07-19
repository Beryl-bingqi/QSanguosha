#include "hegemony.h"
#include "client.h"
#include "engine.h"
#include "carditem.h"
#include "general.h"
#include "room.h"
#include "standard-skillcards.h"


class TripleAngleBladeSkill: public WeaponSkill{
public:
    TripleAngleBladeSkill():WeaponSkill("triple_angle_blade"){
        events << Damage;
    }
    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        QList<ServerPlayer *> targets;
        if(damage.to->isDead())
            return false;
        foreach(ServerPlayer *p, room->getAlivePlayers()){
            if(damage.to->distanceTo(p) == 1)
                targets << p;
        }
        if(targets.isEmpty())
            return false;

        if(damage.card && damage.card->inherits("Slash") && !damage.transfer && !damage.chain && !player->isKongcheng()){
            if(room->askForSkillInvoke(player, objectName(), data)){
                ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());
                if(room->askForDiscard(player, objectName(), 1, 1)){
                    DamageStruct damage;
                    damage.card = NULL;
                    damage.from = player;
                    damage.to = target;
                    room->damage(damage);
                }
            }
        }
        return false;
    }
};


TripleAngleBlade::TripleAngleBlade(Suit suit, int number):Weapon(suit, number, 3){
    setObjectName("triple_angle_blade");
    skill = new TripleAngleBladeSkill;
}

WuSixCard::WuSixCard(){
}

bool WuSixCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select == Self)
        return false;

    return true;
}

void WuSixCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->gainMark("@sword");
}

class WuSixSwordsViewAsSkill: public ZeroCardViewAsSkill{
public:
    WuSixSwordsViewAsSkill():ZeroCardViewAsSkill("wu_six_swords"){
    }

    virtual const Card *viewAs() const{
        return new WuSixCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@wusix";
    }
};

class WuSixSwordsSkill: public WeaponSkill{
public:
    WuSixSwordsSkill():WeaponSkill("wu_six_swords"){
        events << PhaseChange;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        if(player->getPhase() == Player::Finish){
            QList<ServerPlayer *> players = room->getAllPlayers();
            foreach(ServerPlayer *player, players){
                if(player->getMark("@sword") > 0)
                    player->loseMark("@sword");
            }

            room->askForUseCard(player, "@@wusix", "@wusix-card");
        }
        return false;
    }
};

void WuSixSwords::onMove(const CardMoveStruct &move) const{
    if(move.from_place == Player::Equip){
        ServerPlayer* from = (ServerPlayer*) move.from;
        Room *room = from->getRoom();
        QList<ServerPlayer *> players = room->getAllPlayers();
        foreach(ServerPlayer *player, players){
            if(player->getMark("@sword") > 0)
                player->loseMark("@sword");
        }
    }
    if(move.to_place == Player::Equip){
        ServerPlayer *to = (ServerPlayer*)move.to;
        Room *room = to->getRoom();
        room->askForUseCard(to, "@@wusix", "@wusix-card");
    }
}

WuSixSwords::WuSixSwords(Suit suit, int number):Weapon(suit, number, 2){
    setObjectName("wu_six_swords");
    skill = new WuSixSwordsSkill;
    attach_skill = true;
}

AttackExhausted  ::AttackExhausted (Card::Suit suit, int number)
    :TrickCard(suit, number, true)
{
    setObjectName("attack_exhausted");
}

QString AttackExhausted ::getSubtype() const{
    return "mutiple_target_trick";
}

bool AttackExhausted ::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return true;
}

bool AttackExhausted ::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
        return targets.length() > 0;
}

void AttackExhausted ::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    effect.to->drawCards(2);
    room->askForDiscard(effect.to, "attack_exhausted", 2, 2, false, true);
}

KnowAll::KnowAll(Card::Suit suit, int number)
    :SingleTargetTrick(suit, number, false)
{
    setObjectName("knowall");
}


bool KnowAll::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(targets.length() >= 1)
        return false;
    if(to_select == Self)
        return false;
    return true;
}

bool KnowAll::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const{
    if(getSkillName() == "guhuo" || getSkillName() == "qice")
        return targets.length() == 1;
    else
        return targets.length() <= 1;
}

void KnowAll::onUse(Room *room, const CardUseStruct &card_use) const{
    if(card_use.to.isEmpty()){
        room->throwCard(this);
        card_use.from->playCardEffect("@recast");
        card_use.from->drawCards(1);
    }else
        TrickCard::onUse(room, card_use);
}

void KnowAll::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->showAllCards(effect.to, effect.from);
}

BefriendDistant::BefriendDistant(Card::Suit suit, int number)
    :SingleTargetTrick(suit, number, true)
{
    setObjectName("befriend_distant");
}

bool BefriendDistant::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    QList<const Player *> players = Self->getSiblings();
    int farest = 0;
    foreach(const Player *player, players){
        int distance = Self->distanceTo(player);
        farest = qMax(farest, distance);
    }
    return targets.isEmpty() && Self->distanceTo(to_select) == farest;
}

void BefriendDistant::onEffect(const CardEffectStruct &effect) const{
    effect.to->drawCards(1);
    effect.from->drawCards(3);
}


class Xiaoguo:public TriggerSkill{
public:
    Xiaoguo():TriggerSkill("xiaoguo"){
        events << PhaseChange;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *yuejin = room->findPlayerBySkillName(objectName());
        if(yuejin == NULL || yuejin->objectName() == room->getCurrent()->objectName())
            return false;
        int basicnum = 0;
        foreach(const Card *c, yuejin->getHandcards()){
            if(c->inherits("BasicCard")){
                basicnum++;
            }
        }
        if(basicnum == 0)
            return false;
        if(player->getPhase() == Player::Finish && room->askForSkillInvoke(yuejin, objectName())){
            if(room->askForCard(yuejin, ".Basic", "@xiaoguo", QVariant(), CardDiscarded))
                if(!room->askForCard(room->getCurrent(), ".Equip", "@xiaoguo-discard", QVariant(), CardDiscarded)){
                    DamageStruct damage;
                    damage.card = NULL;
                    damage.from = yuejin;
                    damage.to = room->getCurrent();
                    room->damage(damage);
                }
        }
        return false;
    }
};


class Shushen: public TriggerSkill{
public:
    Shushen():TriggerSkill("shushen"){
        events << HpRecover;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
        if(room->askForSkillInvoke(player,objectName())){
            QList<ServerPlayer *> targets = room->getOtherPlayers(player);
            ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());
            target->drawCards(1);
        }
        return false;
    }
};

class Shenzhi:public PhaseChangeSkill{
public:
    Shenzhi():PhaseChangeSkill("shenzhi"){
    }

    virtual bool onPhaseChange(ServerPlayer *ganfuren) const{
        Room *room = ganfuren->getRoom();
        if(ganfuren->getPhase() == Player::Start && room->askForSkillInvoke(ganfuren,objectName())){
            int handcard_num = ganfuren->getHandcardNum();
            room->askForDiscard(ganfuren, objectName(), handcard_num, handcard_num);
            if(handcard_num >= ganfuren->getHp()){
                RecoverStruct recover;
                recover.who = ganfuren;
                room->recover(ganfuren, recover);
            }
        }
        return false;
    }
};

class Duoshi:public OneCardViewAsSkill{
public:
    Duoshi():OneCardViewAsSkill("duoshi"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return true;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return false;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getFilteredCard()->isRed() && !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        AttackExhausted *await = new AttackExhausted(card->getSuit(), card->getNumber());
        await->addSubcard(card->getId());
        await->setSkillName(objectName());
        return await;
    }
};

class Duanbing: public TriggerSkill{
public:
    Duanbing():TriggerSkill("duanbing"){
        events << CardUsed << CardFinished;
    }

    virtual bool trigger(TriggerEvent event, Room *room, ServerPlayer *player, QVariant &data) const{
        CardUseStruct use = data.value<CardUseStruct>();
        if(event == CardUsed){
            if(use.card->inherits("Slash"))
            {
                QList<ServerPlayer *> targets;
                QList<ServerPlayer *> players = room->getOtherPlayers(player);
                foreach(ServerPlayer *p, use.to){
                    players.removeOne(p);
                }
                foreach(ServerPlayer *p, players){
                    if(player->distanceTo(p) == 1)
                        targets << p;
                }
                if(targets.empty())
                    return false;
                if(!player->hasFlag("DuanbingInvoke") && room->askForSkillInvoke(player,objectName())){
                    ServerPlayer *target = room->askForPlayerChosen(player, targets, objectName());
                    if(target){
                        room->setPlayerFlag(player, "DuanbingInvoke");
                        use.to.append(target);
                        data = QVariant::fromValue(use);
                        //room->useCard(card_use);
                    }
                }
            }
        }
        else{
            if(use.card->inherits("Slash") && player->hasFlag("DuanbingInvoke"))
                room->setPlayerFlag(player, "-DuanbingInvoke");
        }
        return false;
    }
};

FenxunCard::FenxunCard(){
    once = true;
    will_throw = true;
}

bool FenxunCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self ;
}

void FenxunCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    room->setPlayerFlag(effect.to, "FenxunTarget");
    room->setFixedDistance(effect.from, effect.to, 1);
}

class FenxunViewAsSkill:public OneCardViewAsSkill{
public:
    FenxunViewAsSkill():OneCardViewAsSkill("fenxun"){
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return !player->hasUsed("FenxunCard");;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return false;
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        const Card *card = card_item->getCard();
        FenxunCard *first = new FenxunCard;
        first->addSubcard(card->getId());
        first->setSkillName(objectName());
        return first;
    }
};

class Fenxun: public PhaseChangeSkill{
public:
    Fenxun():PhaseChangeSkill("fenxun"){
        view_as_skill = new FenxunViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *target) const{
        if(target->getPhase() == Player::Finish){
            Room *room = target->getRoom();
            QList<ServerPlayer *> players = room->getAlivePlayers();

            foreach(ServerPlayer *player, players){
                if(player->hasFlag("FenxunTarget")){
                    room->setPlayerFlag(player, "-FenxunTarget");
                    room->setFixedDistance(target, player, -1);
                }
            }
        }

        return false;
    }
};

class Mingshi: public TriggerSkill{
public:
    Mingshi():TriggerSkill("mingshi"){
        events << DamageInflicted;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.from){
            QString choice = room->askForChoice(damage.from, objectName(), "yes+no");
            if(choice == "no"){
                LogMessage log;
                log.type = "#Mingshi";
                log.from = player;
                log.arg = QString::number(damage.damage);
                log.arg2 = objectName();
                room->sendLog(log);

                damage.damage--;
                if(damage.damage < 1)
                    return true;
                data = QVariant::fromValue(damage);
            }
            else{
                room->showAllCards(damage.from);
            }
        }
        return false;
    }
};

class Lirang: public TriggerSkill{
public:
    Lirang():TriggerSkill("lirang"){
        events << CardDiscarded;
    }

    virtual bool trigger(TriggerEvent , Room* room, ServerPlayer *kongrong, QVariant &data) const{
        const Card *card = data.value<CardStar>();

        if(room->askForSkillInvoke(kongrong, "lirang", data)){
            ServerPlayer *target = room->askForPlayerChosen(kongrong, room->getOtherPlayers(kongrong), objectName());
            target->obtainCard(card);
        }
        return false;
    }
};

class Sijian: public TriggerSkill{
public:
    Sijian():TriggerSkill("sijian"){
        events << CardLost;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *tianfeng, QVariant &data) const{
        if (tianfeng == NULL) return false;
        if(tianfeng->isKongcheng()){
            CardMoveStar move = data.value<CardMoveStar>();

            if(move->from_place == Player::Hand){
                QList<ServerPlayer *> other_players = room->getOtherPlayers(tianfeng);
                foreach(ServerPlayer *p, other_players){
                    if(p->hasFlag("changingh") && p->getCards("e").empty())
                        other_players.removeOne(p);
                    else if(p->isNude())
                        other_players.removeOne(p);
                }
                if(!other_players.empty() && room->askForSkillInvoke(tianfeng, objectName())){

                    ServerPlayer *target = room->askForPlayerChosen(tianfeng, other_players, objectName());
                    if(target->hasFlag("changingh")){
                        int card_id = room->askForCardChosen(tianfeng, target, "e", objectName());
                        room->throwCard(card_id,target);
                    }
                    else{
                        int card_id = room->askForCardChosen(tianfeng, target, "he", objectName());
                        room->throwCard(card_id,target);
                    }
                }
            }
        }

        return false;
    }
};

class Suishi: public TriggerSkill{
public:
    Suishi():TriggerSkill("suishi"){
        events << DamageDone << Dying << Death << DamageComplete;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return !target->hasSkill(objectName());
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &data) const{
        ServerPlayer *tianfeng = room->findPlayerBySkillName(objectName());
        if(!tianfeng)
            return false;
        ServerPlayer *target = NULL;
        if(event == DamageDone){
            DamageStruct damage = data.value<DamageStruct>();
            if(damage.from)
                room->setPlayerFlag(damage.from, "SuishiTarget");
        }
        if(event == Dying && tianfeng){
            foreach(ServerPlayer *other, room->getAlivePlayers()){
                if(other->hasFlag("SuishiTarget")){
                    target = other;
                    break;
                }
            }
            if(target && room->askForChoice(target, "suishi1", "draw+no") == "draw")
                tianfeng->drawCards(1);
        }
        else if(event == Death && tianfeng){
            foreach(ServerPlayer *other, room->getAlivePlayers()){
                if(other->hasFlag("SuishiTarget")){
                    target = other;
                    break;
                }
            }
            if(target && room->askForChoice(target, "suishi2", "damage+no") == "damage")
                room->loseHp(tianfeng);
        }
        else if(event == DamageComplete){
            foreach(ServerPlayer *other, room->getAlivePlayers()){
                if(other->hasFlag("SuishiTarget")){
                    room->setPlayerFlag(other, "-SuishiTarget");
                    break;
                }
            }
        }
        return false;
    }
};

ShuangrenCard::ShuangrenCard(){

}

bool ShuangrenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && !to_select->isKongcheng() && to_select != Self;
}

void ShuangrenCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    bool success = effect.from->pindian(effect.to, "shuangren", this);
    if(success){
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *target, room->getAlivePlayers()){
            if(effect.from->canSlash(target, false))
                targets << target;
        }

        ServerPlayer *target = room->askForPlayerChosen(effect.from, targets, "shuangren-slash");

        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->setSkillName("shuangren");

        CardUseStruct card_use;
        card_use.card = slash;
        card_use.from = effect.from;
        card_use.to << target;
        room->useCard(card_use, false);
    }else{
        QList<Player::Phase> phases = effect.from->getPhases();
        effect.from->play(phases);
    }
}

class ShuangrenViewAsSkill: public OneCardViewAsSkill{
public:
    ShuangrenViewAsSkill():OneCardViewAsSkill("shuangren"){

    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@shuangren";
    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return !to_select->isEquipped();
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new ShuangrenCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class Shuangren:public PhaseChangeSkill{
public:
    Shuangren():PhaseChangeSkill("shuangren"){
        view_as_skill = new ShuangrenViewAsSkill;
    }

    virtual bool onPhaseChange(ServerPlayer *jiling) const{
        if(jiling->getPhase() == Player::Play){
            Room *room = jiling->getRoom();
            bool can_invoke = false;
            QList<ServerPlayer *> other_players = room->getOtherPlayers(jiling);
            foreach(ServerPlayer *player, other_players){
                if(!player->isKongcheng()){
                    can_invoke = true;
                    break;
                }
            }

            if(can_invoke)
                room->askForUseCard(jiling, "@@shuangren", "@shuangren-card");
        }

        return false;
    }
};

HuoshuiCard::HuoshuiCard(){
}

bool HuoshuiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    if(to_select == Self)
        return false;
    return targets.length() < Self->getMark("huoshui");
}

void HuoshuiCard::onEffect(const CardEffectStruct &effect) const{
    effect.to->gainMark("@huoshui");
}

class HuoshuiViewAsSkill: public ZeroCardViewAsSkill{
public:
    HuoshuiViewAsSkill():ZeroCardViewAsSkill("huoshui"){
    }

    virtual const Card *viewAs() const{
        return new HuoshuiCard;
    }

protected:
    virtual bool isEnabledAtPlay(const Player *player) const{
        return false;
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const{
        return  pattern == "@@huoshui";
    }
};


class HuoshuiEX: public TriggerSkill{
public:
    HuoshuiEX():TriggerSkill("exhuoshui"){
        events << PhaseChange;
        view_as_skill = new HuoshuiViewAsSkill;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *zoushi, QVariant &data) const{

        if(zoushi->getPhase() == Player::Start){
            int point = qrand() % 6;
            LogMessage log;
            log.type = "#Huoshuipoint";
            log.from = zoushi;
            log.arg = QString::number(point+1);
            room->sendLog(log);
            if(point > 0){
                zoushi->gainMark("huoshui", point);
                    room->askForUseCard(zoushi, "@@huoshui", "@huoshui-card");
            }
        }
        else if(zoushi->getPhase() == Player::Finish){
            foreach(ServerPlayer* p, room->getOtherPlayers(zoushi))
                if(p->getMark("@huoshui") > 0)
                    p->loseMark("@huoshui");
                zoushi->setMark("huoshui", 0);
        }
        return false;
    }
};

class Huoshui: public TriggerSkill{
public:
    Huoshui():TriggerSkill("huoshui"){
        events << PhaseChange;
        view_as_skill = new HuoshuiViewAsSkill;
    }

    virtual int getPriority() const{
        return 3;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *zoushi, QVariant &data) const{

        if(zoushi->getPhase() == Player::Play){
            int point = 0;
            foreach(const Card *card, zoushi->getHandcards()){
                if(card->isRed() && room->askForChoice(zoushi, "huoshui", "yes+no") == "yes"){
                    room->showCard(zoushi, card->getEffectiveId());
                    point++;
                }
            }

            if(point > 0){
                zoushi->gainMark("huoshui", point);
                room->askForUseCard(zoushi, "@@huoshui", "@huoshui-card");
            }
        }
        else if(zoushi->getPhase() == Player::Finish){
            foreach(ServerPlayer* p, room->getOtherPlayers(zoushi))
                if(p->getMark("@huoshui") > 0)
                    p->loseMark("@huoshui");
            zoushi->setMark("huoshui", 0);
        }
        return false;
    }
};

QingchengCard::QingchengCard(){
    will_throw = true;
}

bool QingchengCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return targets.isEmpty() && to_select != Self;
}

void QingchengCard::onEffect(const CardEffectStruct &effect) const{
    Room *room = effect.from->getRoom();
    effect.to->gainMark("@qingcheng");
    room->setPlayerFlag(effect.to, "Qingcheng");
    LogMessage log;
    log.type = "#QingchengLockSkills";
    log.from = effect.from;
    log.to << effect.to;
    log.arg = "qingcheng";
    room->sendLog(log);
}

class Qingcheng: public OneCardViewAsSkill{
public:
    Qingcheng():OneCardViewAsSkill("qingcheng"){

    }

    virtual bool viewFilter(const CardItem *to_select) const{
        return to_select->getCard()->inherits("EquipCard");
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return true;
    }

    virtual const Card *viewAs(CardItem *card_item) const{
        Card *card = new QingchengCard;
        card->addSubcard(card_item->getFilteredCard());
        return card;
    }
};

class QingchengClear: public TriggerSkill{
public:
    QingchengClear():TriggerSkill("#qingcheng"){
        events << PhaseChange << Death;
    }

    virtual bool triggerable(const ServerPlayer *target) const{
        return true;
    }

    virtual bool trigger(TriggerEvent event, Room* room, ServerPlayer *player, QVariant &) const{
        if(event == PhaseChange && player->getPhase() == Player::Start && player->hasFlag("Qingcheng")){
            room->setPlayerFlag(player, "-Qingcheng");
            if(player->getMark("@qingcheng") > 0)
                player->loseMark("@qingcheng");
            LogMessage log;
            log.type = "#QingchengRecover";
            log.to << player;
            room->sendLog(log);
        }
        else if(event == Death && player->hasSkill("qingcheng")){
            foreach(ServerPlayer *p,room->getOtherPlayers(player)){
                if(p->hasFlag("Qingcheng")){
                    room->setPlayerFlag(p, "-Qingcheng");
                    if(p->getMark("@qingcheng") > 0)
                        p->loseMark("@qingcheng");
                    LogMessage log;
                    log.type = "#QingchengRecover";
                    log.to << p;
                    room->sendLog(log);
                }
            }
            // clear @huoshui either
            if(room->getCurrent()->objectName() == player->objectName())
                foreach(ServerPlayer *p,room->getOtherPlayers(player))
                    p->loseMark("@huoshui");
        }
        return false;
    }
};


class Xiongyi: public ZeroCardViewAsSkill{
public:
    Xiongyi():ZeroCardViewAsSkill("xiongyi"){
        frequency = Limited;
    }

    virtual const Card *viewAs() const{
        return new XiongyiCard;
    }

    virtual bool isEnabledAtPlay(const Player *player) const{
        return player->getMark("@arise") >= 1;
    }
};

class XiongyiRecover: public TriggerSkill{
public:
    XiongyiRecover():TriggerSkill("#xiongyi"){
        events <<  CardFinished;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
        if(player->getMark("@arise") < 1 && player->getMark("xiongyi") > 0){
            if(player->getMark("xiongyi") <= (room->getAlivePlayers().length())/2){
                RecoverStruct recover;
                recover.who = player;
                room->recover(player, recover);
                player->setMark("xiongyi", 0);
            }
        }
        return false;
    }
};

XiongyiCard::XiongyiCard(){
}

bool XiongyiCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const{
    return true;
}

void XiongyiCard::onEffect(const CardEffectStruct &effect) const{
    if(effect.from->getMark("@arise") >= 1)
        effect.from->loseMark("@arise");
    effect.to->drawCards(3);
    effect.from->addMark("xiongyi");

}

class Kuangfu: public TriggerSkill{
public:
    Kuangfu():TriggerSkill("kuangfu"){
        events << Damage;
    }

    virtual bool trigger(TriggerEvent, Room* room, ServerPlayer *panfeng, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        ServerPlayer *target = damage.to;
        if(damage.card && damage.card->inherits("Slash") && target->hasEquip()){
            int card_id = room->askForCardChosen(panfeng, target , "e", "kuangfu");
            const Card *card = Sanguosha->getCard(card_id);
            QString choice = room->askForChoice(panfeng, "kuangfu", "throw+move");
            int equip_index = -1;
            const EquipCard *equip = qobject_cast<const EquipCard *>(card);
            equip_index = static_cast<int>(equip->location());

            if(equip_index > -1 && panfeng->getEquip(equip_index) == NULL && choice == "move"){
                room->moveCardTo(card, panfeng, Player::Equip);
            }
            else{
                room->throwCard(card,target);
            }
        }

        return false;
    }
};

class MingshiEX: public TriggerSkill{
public:
    MingshiEX():TriggerSkill("exmingshi"){
        events << DamageInflicted;
        frequency = Compulsory;
    }

    virtual bool trigger(TriggerEvent , Room *room, ServerPlayer *player, QVariant &data) const{
        DamageStruct damage = data.value<DamageStruct>();
        if(damage.from && damage.from->getEquips().length() <= damage.to->getEquips().length()){
                LogMessage log;
                log.type = "#Mingshi";
                log.from = player;
                log.arg = QString::number(damage.damage);
                log.arg2 = objectName();
                room->sendLog(log);

                damage.damage--;
                if(damage.damage < 1)
                    return true;
                data = QVariant::fromValue(damage);
        }
        return false;
    }
};



HegemonyCardPackage::HegemonyCardPackage()
    :Package("hegemony_card")
{
    QList<Card *> cards;

    // spade
    /*cards << new GudingBlade(Card::Spade, 1)
            << new Vine(Card::Spade, 2)
            << new Analeptic(Card::Spade, 3)
            << new ThunderSlash(Card::Spade, 4)
            << new ThunderSlash(Card::Spade, 5)
            << new ThunderSlash(Card::Spade, 6)
            << new ThunderSlash(Card::Spade, 7)
            << new ThunderSlash(Card::Spade, 8)
            << new Analeptic(Card::Spade, 9)
            << new SupplyShortage(Card::Spade,10)
            << new IronChain(Card::Spade, 11)
            << new IronChain(Card::Spade, 12)
            << new Nullification(Card::Spade, 13);
*/
    // club
    cards << new KnowAll(Card::Club, 3)
            /*<< new Vine(Card::Club, 2)
            << new Analeptic(Card::Club, 3)
            << new SupplyShortage(Card::Club, 4)
            << new ThunderSlash(Card::Club, 5)
            << new ThunderSlash(Card::Club, 6)
            << new ThunderSlash(Card::Club, 7)
            << new ThunderSlash(Card::Club, 8)
            << new Analeptic(Card::Club, 9)
            << new IronChain(Card::Club, 10)
            << new IronChain(Card::Club, 11)
            << new IronChain(Card::Club, 12)*/
            << new KnowAll(Card::Club, 4);

    // heart
    cards << new BefriendDistant(Card::Heart, 9)
            /*<< new FireAttack(Card::Heart, 2)
            << new FireAttack(Card::Heart, 3)
            << new FireSlash(Card::Heart, 4)
            << new Peach(Card::Heart, 5)
            << new Peach(Card::Heart, 6)
            << new FireSlash(Card::Heart, 7)
            << new Jink(Card::Heart, 8)
            << new Jink(Card::Heart, 9)
            << new FireSlash(Card::Heart, 10)
            << new Jink(Card::Heart, 11)
            << new Jink(Card::Heart, 12)*/
            << new AttackExhausted(Card::Heart, 11);

    // diamond
    cards << new AttackExhausted(Card::Diamond, 4)
            /*<< new Peach(Card::Diamond, 2)
            << new Peach(Card::Diamond, 3)
            << new FireSlash(Card::Diamond, 4)
            << new FireSlash(Card::Diamond, 5)
            << new Jink(Card::Diamond, 6)
            << new Jink(Card::Diamond, 7)
            << new Jink(Card::Diamond, 8)
            << new Analeptic(Card::Diamond, 9)
            << new Jink(Card::Diamond, 10)*/
            << new WuSixSwords(Card::Diamond, 6)
            << new TripleAngleBlade(Card::Diamond, 12);


    foreach(Card *card, cards)
        card->setParent(this);

    type = CardPack;
    skills << new WuSixSwordsViewAsSkill;
    addMetaObject<WuSixCard>();
}

ADD_PACKAGE(HegemonyCard)

HegemonyPackage::HegemonyPackage()
    :Package("hegemony")
    {
        General *yuejin = new General(this, "yuejin", "wei");
        yuejin->addSkill(new Xiaoguo);

        General *ganfuren = new General(this, "ganfuren", "shu", 3, false);
        ganfuren->addSkill(new Shushen);
        ganfuren->addSkill(new Shenzhi);

        General *sp_luxun = new General(this, "sp_luxun", "wu", 3);
        sp_luxun->addSkill("qianxun");
        sp_luxun->addSkill(new Duoshi);

        General *dingfeng = new General(this, "dingfeng", "wu");
        dingfeng->addSkill(new Duanbing);
        dingfeng->addSkill(new Fenxun);

        General *kongrong = new General(this, "kongrong", "qun", 3);
        kongrong->addSkill(new Mingshi);
        kongrong->addSkill(new Lirang);

        General *tianfeng = new General(this, "tianfeng", "qun", 3);
        tianfeng->addSkill(new Sijian);
        tianfeng->addSkill(new Suishi);

        General *jiling = new General(this, "jiling", "qun", 4);
        jiling->addSkill(new Shuangren);

        General *zoushi = new General(this, "zoushi", "qun", 3, false);
        zoushi->addSkill(new Huoshui);
        zoushi->addSkill(new Qingcheng);
        zoushi->addSkill(new QingchengClear);
        related_skills.insertMulti("qingcheng", "#qingcheng");

        General *mateng = new General(this, "mateng", "qun");
        mateng->addSkill("mashu");
        mateng->addSkill(new MarkAssignSkill("@arise", 1));
        mateng->addSkill(new Xiongyi);
        mateng->addSkill(new XiongyiRecover);
        related_skills.insertMulti("xiongyi", "#xiongyi");

        General *panfeng = new General(this, "panfeng", "qun");
        panfeng->addSkill(new Kuangfu);

        General *kongwenju = new General(this, "kongwenju", "qun", 3);
        kongwenju->addSkill(new MingshiEX);
        kongwenju->addSkill("lirang");

        General *sp_zoushi = new General(this, "sp_zoushi", "qun", 3, false);
        sp_zoushi->addSkill(new HuoshuiEX);
        sp_zoushi->addSkill("qingcheng");
        sp_zoushi->addSkill("#qingcheng");

        addMetaObject<FenxunCard>();
        addMetaObject<ShuangrenCard>();
        addMetaObject<HuoshuiCard>();
        addMetaObject<QingchengCard>();
        addMetaObject<XiongyiCard>();
        }

    ADD_PACKAGE(Hegemony)






