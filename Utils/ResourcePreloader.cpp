#include "OrbsPluginPCH.h"
#include "ResourcePreloader.h"

void ResourcePreloader::preloadMenusTextures()
{
	//splashscreens
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\splash\\headphones_required_512.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\splash\\paralyze_logo.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\splash\\paralyze_title.png");

	//intro_menu
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\bubble_bg.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\bubble_heart.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\circle1.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\circle2.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\credits_bg_color.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\credits_bg_contour.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\disabled_button.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\enabled_button.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\options_menu_bg_slice.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\options_menu_bracket.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_credits.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_music_off.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_music_on.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_options.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_reset.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_sound_off.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_sound_on.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_trophy.png");

	//title
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\title\\title.png");

	//levels
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\levels_menu\\itemlist_arrow.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\levels_menu\\itemlist_arrow2.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\levels_menu\\levels_back_btn.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\levels_menu\\levels_fountain_bubble.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\levels_menu\\levels_fountain_circle.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\levels_menu\\levels_list_item_bg.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\levels_menu\\levels_list_item_lock.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\levels_menu\\levels_start_btn.png");

	//achievements
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\achievements\\achievement_all_primary.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\achievements\\achievement_all_secondary.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\achievements\\achievement_kill_1.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\achievements\\achievement_kill_2.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\achievements\\achievement_kill_3.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\achievements\\achievement_locked.png");
}

void ResourcePreloader::preloadGameTextures()
{
	//action panel
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\action_panel\\move_btn.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\action_panel\\rally_point_btn.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\action_panel\\build_btn.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\action_panel\\sub_menu_validate_btn.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\action_panel\\sub_menu_undo_btn.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\action_panel\\sub_menu_delete_btn.png");

	//core panel
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\generation_meta_orb.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\defense_meta_orb.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\assault_meta_orb.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\core_panel_support_body.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\core_panel_support_left_side.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\core_panel_support_right_side.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\gauge_bg.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\gauge_grid.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\core_panel_update_generation.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\core_panel_update_defense.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\core_panel_update_assault.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\core_panel_update_back_btn.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\core_panel\\core_panel_update_validate_btn.png");

	//info panel
	VTextureObject* bgTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\info_panel\\info_panel_bg_texture.png");

	//misc panel
	VTextureObject* pBgBodyTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\misc_panel\\misc_panel_bg_body.png");
	VTextureObject* pBgLeftSideTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\misc_panel\\misc_panel_bg_left_side.png");
	VTextureObject* pBgRightSideTexture = Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\misc_panel\\misc_panel_bg_right_side.png");

	//interface panel
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\misc\\spinning_wheel.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\interface_panel\\close_btn.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\interface_panel\\objectives_btn_light.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\interface_panel\\help_btn_light.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\interface_panel\\pause_btn_light.png");

	//defeat victory menu
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\defeat_victory\\defeat_bottom_banner_line.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\defeat_victory\\defeat_top_banner_bottom_line.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\defeat_victory\\defeat_top_banner_top_line.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\defeat_victory\\victory_bottom_banner_line.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\defeat_victory\\victory_top_banner_bottom_line.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\defeat_victory\\victory_top_banner_top_line.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\defeat_victory\\blue_circle_button.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\defeat_victory\\red_circle_button.png");

	//halos
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\halos\\halo_arrow.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\halos\\path_halo.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\halos\\path_point.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\halos\\selection_halo.png");

	//pause menu
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\pause_menu\\overlay_save.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_reset.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_credits.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_music_on.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_music_off.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_sound_on.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\intro_menu\\overlay_sound_off.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\pause_menu\\game_speed_bg.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\pause_menu\\game_speed_knob.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\pause_menu\\popup_btn.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\pause_menu\\yes_no_window_bracket.png");

	//objectives menu
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\objectives_menu\\hexagon.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\objectives_menu\\point_on.png");
	Vision::TextureManager.Load2DTexture("GUI\\Textures\\hdpi\\objectives_menu\\point_off.png");

	//scene objects
	Vision::TextureManager.Load2DTexture("Textures\\bridge_texture.png");
	Vision::TextureManager.Load2DTexture("Textures\\potential_bridge_texture.png");
	Vision::TextureManager.Load2DTexture("Textures\\path_texture.png");
	Vision::TextureManager.Load2DTexture("Textures\\orb_glow.png");
	Vision::TextureManager.Load2DTexture("Textures\\space.png");
}