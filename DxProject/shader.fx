//--------------------------------------------------------------------------------------
// Вершинный шейдер
//--------------------------------------------------------------------------------------
float4 VS( float4 Pos : POSITION ) : SV_POSITION
{
	Pos.x *= 0.5;
	// Оставляем координаты точки без изменений
	return Pos;
}

//--------------------------------------------------------------------------------------
// Пиксельный шейдер
//--------------------------------------------------------------------------------------
float4 PS( float4 Pos : SV_POSITION ) : SV_Target
{
	float fLimiter = 500;
	float dist = Pos.x * Pos.x + Pos.y * Pos.y;
	dist = (dist % fLimiter) / fLimiter;
	// Возвращаем желтый цвет, непрозрачный (альфа == 1, альфа-канал не включен).
	return float4(dist, 0.0f, dist, 1.0f);
}