#include "Enemies/EnemySpawnerSubsystem.h"
#include "Enemies/EmberEnemyBase.h"

void UEnemySpawnerSubsystem::RegisterEnemy(AEmberEnemyBase* Enemy)
{
	if (Enemy)
	{
		LiveEnemies.AddUnique(Enemy);
	}
}

void UEnemySpawnerSubsystem::UnregisterEnemy(AEmberEnemyBase* Enemy)
{
	LiveEnemies.Remove(Enemy);
}
