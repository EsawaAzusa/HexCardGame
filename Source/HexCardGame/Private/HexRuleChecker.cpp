#include "HexRuleChecker.h"
#include "HexCardState.h"

bool UHexRuleChecker::PlayCardLegalCheck(int PlayerID, int CardInstanceID, int HexQ, int HexR)
{
	/* 七条规则：
	 *非己方回合不能出牌
	 *不能替对方出牌
	 *必须在棋盘上出牌
	 *不能覆盖出牌
	 *必须在己方相邻Range内出牌
	 * 必须在对方阵线前出牌
	 * BasePower必须超越对方
	 */
	FCardState CardState = OwnerHexCardState -> GetCardInstancebyID(CardInstanceID, OwnerHexCardState -> CardStates);
	
	//1，
	if (OwnerHexCardState -> CurrentTurnPlayerID != CardState.OwnerPlayerID) return false;
	
	//2，
	if (PlayerID != CardState.OwnerPlayerID) return false;
	
	//3，
	if (!(HexQ + HexR >= -3 && HexQ + HexR <=3 && HexQ >= -2 && HexQ <= 2 && HexR >= -3 && HexR <= 3)) return false;
	
	//4，
	if (OwnerHexCardState -> GetCardInstancebyHex(HexQ, HexR, OwnerHexCardState -> CardStates).IsValid()) return false;
	
	//5，
	TArray<FIntPoint> Directions;
	bool FindFriend = false;
	if (CardState.OwnerPlayerID == 0 && HexQ == 0 && HexR == -3) FindFriend  = true;
	if (CardState.OwnerPlayerID == 1 && HexQ == 0 && HexR == 3) FindFriend = true;
	if(CardState.BaseRange == 1)
	{
		Directions =
			{
			FIntPoint(1, 0),
			FIntPoint(-1, 0),
			FIntPoint(0, 1),
			FIntPoint(0, -1),
			FIntPoint(-1, 1),
			FIntPoint(1, -1)
			} ;
	}
	else if (CardState.BaseRange == 2)
	{
		Directions =
		{
			FIntPoint(1, 0),
			FIntPoint(-1, 0),
			FIntPoint(-2, 0),
			FIntPoint(0, 1),
			FIntPoint(0, 2),
			FIntPoint(0, -1),
			FIntPoint(0, -2),
			FIntPoint(-1, 1), 
			FIntPoint(-2, 2), 
			FIntPoint(1, -1),
			FIntPoint(2, -2) 
		};
	};
	for (const FIntPoint& Direction : Directions)
	{
		const int NeigborHexQ = HexQ + Direction.X;
		const int NeigborHexR = HexR + Direction.Y;
		FCardState Find = OwnerHexCardState -> GetCardInstancebyHex(NeigborHexQ, NeigborHexR, OwnerHexCardState -> CardStates);
		if (Find.IsValid() && Find.OwnerPlayerID == CardState.OwnerPlayerID)
		{
			FindFriend = true;
			break;
		}
	};
	if (!FindFriend) return false;
	
	//6，
	const int OffsetHexR = HexQ + 2 * HexR;
	bool FindEnemy = false;
	if (CardState.OwnerPlayerID == 0)
	{
		for (int jdx = -2; jdx < 2; jdx ++)
		{
			for (int idx = OffsetHexR; idx > -6; idx--)
			{
				const int AixalHexQ = jdx;
				const int AixalHexR = (idx - jdx)/2;
				FCardState Find = OwnerHexCardState -> GetCardInstancebyHex(AixalHexQ, AixalHexR, OwnerHexCardState -> CardStates);
				if (Find.IsValid() && Find.OwnerPlayerID == 1)
				{
					FindEnemy = true;
					break;
				}
			}
		}
	}
	else if (CardState.OwnerPlayerID == 1)
	{
		for (int jdx = -2; jdx < 2; jdx ++)
		{
			for (int idx = OffsetHexR; idx < 6; idx++)
			{
				const int AixalHexQ = jdx;
				const int AixalHexR = (idx - jdx)/2;
				FCardState Find = OwnerHexCardState -> GetCardInstancebyHex(AixalHexQ, AixalHexR, OwnerHexCardState -> CardStates);
				if (Find.IsValid() && Find.OwnerPlayerID == 0)
				{
					FindEnemy = true;
					break;
				}
			}
		}
	}
	if (FindEnemy) return false;

	//7,
	
}
