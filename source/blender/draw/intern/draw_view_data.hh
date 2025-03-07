/* SPDX-FileCopyrightText: 2021 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup draw
 *
 * Engine data
 * Structure containing each draw engine instance data.
 */

#pragma once

#define GPU_INFO_SIZE 512 /* IMA_MAX_RENDER_TEXT_SIZE */

namespace blender::draw {
class TextureFromPool;
}  // namespace blender::draw

struct DRWRegisteredDrawEngine;
struct DRWTextStore;
struct DrawEngineType;
struct GPUFrameBuffer;
struct GPUTexture;
struct GPUViewport;
struct ListBase;

struct ViewportEngineData {
  /* Not owning pointer to the draw engine. */
  DRWRegisteredDrawEngine *engine_type;

  /**
   * \brief Memory block that can be freely used by the draw engine.
   * When used the draw engine must implement #DrawEngineType.instance_free callback.
   */
  void *instance_data;

  char info[GPU_INFO_SIZE];

  /* we may want to put this elsewhere */
  DRWTextStore *text_draw_cache;
};

struct ViewportEngineData_Info {
  int fbl_len;
  int txl_len;
  int psl_len;
  int stl_len;
};

/* Buffer and textures used by the viewport by default */
struct DefaultFramebufferList {
  GPUFrameBuffer *default_fb;
  GPUFrameBuffer *overlay_fb;
  GPUFrameBuffer *in_front_fb;
  GPUFrameBuffer *color_only_fb;
  GPUFrameBuffer *depth_only_fb;
  GPUFrameBuffer *overlay_only_fb;
};

struct DefaultTextureList {
  GPUTexture *color;
  GPUTexture *color_overlay;
  GPUTexture *depth;
  GPUTexture *depth_in_front;
};

struct DRWViewData;

/**
 * Creates a view data with all possible engines type for this view.
 *
 * `engine_types` contains #DRWRegisteredDrawEngine.
 */
DRWViewData *DRW_view_data_create(ListBase *engine_types);
void DRW_view_data_free(DRWViewData *view_data);

/* Returns a TextureFromPool stored in the given view data for the pass identified by the given
 * pass name. Engines should call this function for each of the passes needed by the viewport
 * compositor in every redraw, then it should allocate the texture and write the pass data to it.
 * The texture should cover the entire viewport. */
blender::draw::TextureFromPool &DRW_view_data_pass_texture_get(DRWViewData *view_data,
                                                               const char *pass_name);

void DRW_view_data_default_lists_from_viewport(DRWViewData *view_data, GPUViewport *viewport);
void DRW_view_data_texture_list_size_validate(DRWViewData *view_data, const int size[2]);
ViewportEngineData *DRW_view_data_engine_data_get_ensure(DRWViewData *view_data,
                                                         DrawEngineType *engine_type);
void DRW_view_data_use_engine(DRWViewData *view_data, DrawEngineType *engine_type);
void DRW_view_data_reset(DRWViewData *view_data);
void DRW_view_data_free_unused(DRWViewData *view_data);
void DRW_view_data_engines_view_update(DRWViewData *view_data);
double *DRW_view_data_cache_time_get(DRWViewData *view_data);
DefaultFramebufferList *DRW_view_data_default_framebuffer_list_get(DRWViewData *view_data);
DefaultTextureList *DRW_view_data_default_texture_list_get(DRWViewData *view_data);

struct DRWEngineIterator {
  int id, end;
  ViewportEngineData **engines;
};

/* Iterate over used engines of this view_data. */
void DRW_view_data_enabled_engine_iter_begin(DRWEngineIterator *iterator, DRWViewData *view_data);
ViewportEngineData *DRW_view_data_enabled_engine_iter_step(DRWEngineIterator *iterator);

#define DRW_ENABLED_ENGINE_ITER(view_data_, engine_, data_) \
  DRWEngineIterator iterator; \
  ViewportEngineData *data_; \
  DrawEngineType *engine_; \
  DRW_view_data_enabled_engine_iter_begin(&iterator, view_data_); \
  /* WATCH Comma operator trickery ahead! This tests engine_ == nullptr. */ \
  while ((data_ = DRW_view_data_enabled_engine_iter_step(&iterator), \
          engine_ = (data_ != nullptr) ? (DrawEngineType *)data_->engine_type->draw_engine : \
                                         nullptr))
